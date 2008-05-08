//#define XPGTL_DEBUG
#include <xpgtl/xpath.hpp>
#include <json/jsonpp.hpp>
#include <json/xpath.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>

// for Query
#include <boost/shared_ptr.hpp>
#include <vector>

namespace xpgtl {

  template <typename RU>
  struct visit_node_ru {
    typedef RU result_type;
    template <typename T>
    RU operator () (T const& t) const {
      RU ru;
      ru = &t;
      return ru;
    }
    template <typename T>
    static RU go (T const& t) {
      visit_node_ru<RU> nru;
      return rdstl::visit(nru, t);
    }
  };

  template <typename RU>
  struct abstract_iterator {
    virtual ~ abstract_iterator () {}
    virtual void next () = 0;
    virtual bool eq (abstract_iterator const&) const = 0;
    virtual bool n_eq (abstract_iterator const&) const = 0;
    virtual void dereference () = 0;
    RU const& operator * () {
      this->dereference();
      return this->ru_store;
    }
    const RU* operator -> () {
      this->dereference();
      return &this->ru_store;
    }
    abstract_iterator& operator ++ () {
      this->next(); return *this;
    }
    friend bool operator == (abstract_iterator const& L, abstract_iterator const& R) {
      return L.eq(R);
    }
    friend bool operator != (abstract_iterator const& L, abstract_iterator const& R) {
      return L.n_eq(R);
    }

    protected:
    RU ru_store;
  };
  template <typename RU, typename Iterator, typename Tag>
  struct node_iterator : public abstract_iterator<RU> {
    typedef abstract_iterator<RU>           base_type;
    typedef node_iterator<RU,Iterator,Tag>  self_type;
    node_iterator () : iterator() {}
    node_iterator (Iterator const& iter) : iterator(iter) {}
    virtual ~ node_iterator () {}
    virtual void next () {
      ++this->iterator;
    }
    virtual bool eq (base_type const& atr) const {
      return this->iterator == static_cast<const self_type*>(&atr)->iterator;
    }
    virtual bool n_eq (base_type const& atr) const {
      return this->iterator != static_cast<const self_type*>(&atr)->iterator;
    }
    virtual void dereference () {
      this->ru_store = visit_node_ru<RU>::go(*this->iterator);
    }

    Iterator iterator;
  };
  template <typename RU, typename Tag>
  struct node_iterator<RU,void,Tag> : public abstract_iterator<RU> {
    typedef abstract_iterator<RU>       base_type;
    virtual ~ node_iterator () {}
    virtual void next () {}
    virtual bool eq (base_type const&) const { return true; }
    virtual bool n_eq (base_type const&) const { return false; }
    virtual void dereference () {}
  };

  template <typename String, typename X, typename Tag=xpath<X> >
  struct Query {
    typedef typename rdstl::reference_union<X,Tag>::type    ru_type;
    typedef boost::shared_ptr<abstract_iterator<ru_type> >  sa_iter_t;

    struct item {
      item () : node() {}
      template <typename T>
      item (T const& t) {
        this->alternate = axis_t::unknown;
        this->index = 0;
        this->initialize(t);
      }
      template <typename T>
      item (T const* t) {
        this->initialize(*t);
      }
      template <typename T>
      typename boost::disable_if<rdstl::has_children<T,Tag> >::type
      initialize (T const& t) {
        typedef node_iterator<ru_type,void,Tag> node_iter;
        this->tag_name = tag(t, Tag());
        this->node = &t;
        this->begin = sa_iter_t(new node_iter());
        this->current = sa_iter_t(new node_iter());
        this->end = sa_iter_t(new node_iter());
      }
      template <typename T>
      typename boost::enable_if<rdstl::has_children<T,Tag> >::type
      initialize (T const& t) {
        typedef typename bel::iterator<T,Tag>::type iter_t;
        typedef node_iterator<ru_type,iter_t,Tag> node_iter;
        this->tag_name = tag(t, Tag());
        this->node = &t;
        iter_t f, l;
        boost::tie(f,l) = rdstl::children(t, Tag());
        this->begin = sa_iter_t(new node_iter(f));
        this->current = sa_iter_t(new node_iter(f));
        this->end = sa_iter_t(new node_iter(l));
      }

      axis_t::name_e alternate;
      std::size_t index;
      std::string tag_name;
      sa_iter_t begin, current, end;
      ru_type   node;
    };
    struct build_item {
      typedef item result_type;
      template <typename T>
      item operator () (T const& t) const {
        return item(t);
      }
      template <typename T>
      static item go (T const& t) {
        build_item bi;
        return rdstl::visit(bi, t);
      }
    };
    struct build_item_ptr {
      typedef item result_type;
      template <typename T>
      item operator () (T const& t) const {
        return item(*t);
      }
      template <typename T>
      static item go (T const& t) {
        build_item_ptr bi;
        return rdstl::visit(bi, t);
      }
    };

    Query (path<String> const& path, X const& x) {
      this->setup(path, x);
    }
    void setup (path<String> const& path, X const& x) {
      this->path = path;
      this->work.clear();
      this->work.push_back(build_item::go(x));
    }
    void next () {
      while (not this->work.empty()) {
        item &it = this->work.back();
        if (this->path.size() <= it.index) {
          std::cout << "FOUND!" << std::endl;
          this->work.pop_back();
          break;
        }
        axis_t const& Axis = this->path[it.index];
        axis_t::name_e name
          = axis_t::unknown == it.alternate
              ? Axis.name : it.alternate;
        switch (name) {
        case axis_t::child: {
            if (*it.current != *it.end) {
              const std::size_t idx = it.index;
              this->work.push_back(build_item_ptr::go(**it.current));
              this->work.back().index = idx;
              this->work.back().alternate = axis_t::self;
              ++*it.current;
            } else
              this->work.pop_back();
          } break;
        case axis_t::descendent: {
          } break;
        case axis_t::self: {
            if (it.tag_name == this->path.test(Axis))
              ++it.index;
            else
              this->work.pop_back();
          } break;
        default:
          std::cout << "WTF? pop back!" << std::endl;
          this->work.pop_back();
          break;
        }
      }
    }

    std::vector<item> work;
    path<String> path;
  };

}

int main (int argc, char *argv[]) {

  std::locale loc("");
  std::wcout.imbue(loc);

  std::string input;
  std::istream_iterator<char> ctr(std::cin), cnd;
  std::copy(ctr, cnd, std::back_inserter(input));
  xpgtl::path<std::string> path = xpgtl::path<std::string>(input);
  std::cout << xpgtl::abbreviate << path << std::endl;
  std::cout << xpgtl::long_form << path << std::endl;
  std::cout << std::endl;

  for (++argv; argc > 1; --argc, ++argv) {
    std::cout << *argv << std::endl;
    try {
      std::wifstream wifstr(*argv);
      wifstr.imbue(loc);
      wifstr >> std::noskipws;
      std::istream_iterator<wchar_t,wchar_t> ctr(wifstr);
      std::istream_iterator<wchar_t,wchar_t> cnd;
      JSONpp::json_v json = JSONpp::parse(ctr, cnd);
      xpgtl::Query<std::string,JSONpp::json_v> Q(path, json);
      Q.next();

      std::cout << std::endl;
    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
  }
  return 0;
}
