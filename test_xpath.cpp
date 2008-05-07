//#define XPGTL_DEBUG
#include <xpgtl/xpath.hpp>
#include <json/jsonpp.hpp>
#include <json/xpath.hpp>

#include <boost/shared_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>

#include <list>

namespace xpgtl {

  struct abstract_visitor;
  struct abstract_iterator {
    virtual ~ abstract_iterator () {}
    virtual abstract_iterator* clone () const = 0;
    virtual bool eq (abstract_iterator const&) const = 0;
    virtual bool n_eq (abstract_iterator const&) const = 0;
    virtual void next () = 0;
    abstract_iterator& operator ++ () {
      this->next();
      return *this;
    }
    friend bool operator == (abstract_iterator const& L, abstract_iterator const& R) {
      return L.eq(R);
    }
    friend bool operator != (abstract_iterator const& L, abstract_iterator const& R) {
      return L.n_eq(R);
    }
  };
  struct abstract_visitor {
    virtual ~ abstract_visitor () {}
    virtual std::string get_tag () const = 0;
    virtual std::pair<abstract_iterator*,abstract_iterator*>
    get_children () const = 0;
  };

  template <typename X, typename Tag>
  struct abstract_iterator_T : public abstract_iterator {
    virtual ~ abstract_iterator_T () {}
    abstract_iterator_T () {}
    abstract_iterator_T (X const& x) : iterator(x) {}
    virtual abstract_iterator* clone () const {
      return new abstract_iterator_T<X,Tag>(this->iterator);
    }
    virtual bool eq (abstract_iterator const& atr) const {
      return this->iterator ==
        static_cast<abstract_iterator_T<X,Tag> const*>(&atr)->iterator;
    }
    virtual bool n_eq (abstract_iterator const& atr) const {
      return this->iterator !=
        static_cast<abstract_iterator_T<X,Tag> const*>(&atr)->iterator;
    }
    virtual void next () {
      ++this->iterator;
    }
    X iterator;
  };
  template <typename Tag>
  struct abstract_iterator_T<void,Tag> : public abstract_iterator {
    virtual ~ abstract_iterator_T () {}
    abstract_iterator_T () {}
    template <typename T> abstract_iterator_T (T) {}
    virtual abstract_iterator* clone () const {
      return new abstract_iterator_T<void,Tag>();
    }
    virtual bool eq (abstract_iterator const&) const { return false; }
    virtual bool n_eq (abstract_iterator const&) const { return true; }
    virtual void next () {}
  };
  template <typename X, typename Tag>
  struct abstract_visitor_T : public abstract_visitor {
    abstract_visitor_T (X const& x) : x(&x) {}
    virtual ~ abstract_visitor_T () {
      this->x = 0;
    }
    virtual std::string get_tag () const {
      if (0 != this->x)
        return tag(*this->x, Tag());
      return "";
    }
    virtual std::pair<abstract_iterator*,abstract_iterator*>
    get_children () const {
      return this->get_children(this->x);
    }

    template <typename T>
    typename boost::enable_if<rdstl::has_children<T,Tag>,
      std::pair<abstract_iterator*,abstract_iterator*> >::type
    get_children (T const* t) const {
      typedef typename bel::iterator<T,Tag>::type iterator;
      typedef abstract_iterator_T<iterator,Tag> iter;
      iterator f,l;
      boost::tie(f,l) = rdstl::children(*t, Tag());
      return std::make_pair(new iter(f), new iter(l));
    }
    template <typename T>
    typename boost::disable_if<rdstl::has_children<T,Tag>,
      std::pair<abstract_iterator*,abstract_iterator*> >::type
    get_children (T const*) const {
      typedef abstract_iterator_T<void,Tag> iter;
      return std::make_pair(new iter(), new iter());
    }

    X const* x;
  };

  template <typename X, typename Tag>
  struct make_abstract_visitor {
    typedef abstract_visitor* result_type;
    template <typename T>
    abstract_visitor* operator () (T const& t) const {
      return new abstract_visitor_T<T,Tag>(t);
    }
  };

  template <typename String, typename X>
  struct Query {
    typedef xpath<X> xpath_t;

    struct work_item {
      typedef boost::shared_ptr<abstract_visitor> shared_av;
      typedef boost::shared_ptr<abstract_iterator> shared_itr;

      work_item () {}
      work_item (abstract_visitor *av) : visitor(av) {
        abstract_iterator *first, *last;
        boost::tie(first, last) = av->get_children();
        this->begin = shared_itr(first);
        this->end = shared_itr(last);
        this->child = shared_itr(first->clone());
      }

      shared_av visitor;
      shared_itr begin, end, child;
    };

    Query (path<String> const& path, X const& x) : x(x) {
      this->path = path;
      this->setup();
    }
    void setup () {
      this->work.clear();
      make_abstract_visitor<X,xpath_t> M;
      abstract_visitor *av = rdstl::visit(M, this->x);
      this->work.push_back(work_item(av));
    }
    void next () {
      while (not this->work.empty()) {
        work_item &item = this->work.back();
        if (*item.child != *item.end) {
          ++*item.child;
        } else
          this->work.pop_back();
      }
    }
    bool handle_self (std::size_t idx) {
      const std::string w_tag = this->work.back().visitor->get_tag();
      return w_tag == this->path.test(idx);
    }

    std::list<work_item> work;
    path<String> path;
    X const& x;
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
