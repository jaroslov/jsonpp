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

  struct abstract_iterator {
    virtual ~ abstract_iterator () {}
  };
  template <typename Iterator, typename Tag>
  struct node_iterator : public abstract_iterator {
    node_iterator () : iterator() {}
    node_iterator (Iterator const& iter) : iterator(iter) {}
    virtual ~ node_iterator () {}
    Iterator iterator;
  };
  template <typename Tag>
  struct node_iterator<void,Tag> : public abstract_iterator {
    virtual ~ node_iterator () {}
  };

  template <typename String, typename X, typename Tag=xpath<X> >
  struct Query {
    typedef typename rdstl::reference_union<X,Tag>::type  ru_type;
    typedef boost::shared_ptr<abstract_iterator>          sa_iter_t;

    struct item {
      item () : node() {}
      template <typename T>
      item (T const& t) {
        this->initialize(t);
      }
      template <typename T>
      typename boost::disable_if<rdstl::has_children<T,Tag> >::type
      initialize (T const& t) {
        this->tag = rdstl::tag(t, Tag());
        this->node = &t;
        this->begin = sa_iter_t(new node_iterator<void,Tag>());
        this->current = sa_iter_t(new node_iterator<void,Tag>());
        this->end = sa_iter_t(new node_iterator<void,Tag>());
      }
      template <typename T>
      typename boost::enable_if<rdstl::has_children<T,Tag> >::type
      initialize (T const& t) {
        typedef typename bel::iterator<T,Tag>::type iter_t;
        this->tag = rdstl::tag(t, Tag());
        this->node = &t;
        iter_t f, l;
        boost::tie(f,l) = rdstl::children(t, Tag());
        this->begin = sa_iter_t(new node_iterator<iter_t,Tag>(f));
        this->current = sa_iter_t(new node_iterator<iter_t,Tag>(f));
        this->end = sa_iter_t(new node_iterator<iter_t,Tag>(l));
      }

      std::string tag;
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
        this->work.pop_back();
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
