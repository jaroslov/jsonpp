//#define XPGTL_DEBUG
#include <xpgtl/xpath.hpp>
#include <json/jsonpp.hpp>
#include <json/xpath.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>

#include <list>

namespace xpgtl {

  struct iterator {
    virtual ~ iterator () {}
    virtual void next () const = 0;
    virtual void prev () const = 0;
    virtual bool eq (iterator const*) = 0;
    virtual bool n_eq (iterator const*) = 0;
  };
  template <typename X, typename R>
  struct iterator_T : public iterator {
    X x;
    iterator_T (X const& x) : x(x) {}
    virtual ~ iterator_T () {}
    virtual void next () const { ++this->x; }
    virtual void prev () const { ++this->x; }
    virtual bool eq (iterator const* it) {
      return this->x == static_cast<const iterator_T<X,R>*>(it)->x;
    }
    virtual bool n_eq (iterator const* it) {
      return this->x != static_cast<const iterator_T<X,R>*>(it)->x;
    }
  };
  template <typename R> struct iterator_T<void,R> : public iterator {
    template <typename X> iterator_T (X const& x) {}
    virtual ~ iterator_T () {}
    virtual void next () const {}
    virtual void prev () const {}
    virtual bool eq (iterator const*) { return false; }
    virtual bool n_eq (iterator const*) { return true; }
  };
  struct work {
    virtual ~ work () {}
    virtual std::pair<iterator*,iterator*> children () const = 0;
    virtual std::string tag () const = 0;
    virtual bool proxy () const = 0;
    virtual work* parent () const = 0;
    virtual work* attribute (std::string const&) const = 0;
  };
  template <typename X, typename R>
  struct work_T : public work {
    typedef xpgtl::xpath<R> xpath_t;

    virtual ~ work_T () {}
    work_T (X const& x) : x(&x) {}
    virtual std::pair<iterator*,iterator*> children () const {
      return this->get_children(this->x);
    }
    virtual std::string tag () const = 0;
    virtual bool proxy () const = 0;
    virtual work* parent () const = 0;
    virtual work* attribute (std::string const&) const = 0;

    template <typename Y>
    typename boost::enable_if<rdstl::has_children<Y,xpath_t>,
      std::pair<iterator*,iterator*> >::type
    get_children (Y const*) const {
      typedef typename bel::iterator<Y,xpath_t>::type ch_iter;
      typedef iterator_T<ch_iter,R> iter_type;
      ch_iter f, l;
      boost::tie(f,l) = rdstl::children(*this->x, xpath_t());
      return std::make_pair(new iter_type(f), new iter_type(l));
    }
    template <typename Y>
    typename boost::disable_if<rdstl::has_children<Y,xpath_t>,
      std::pair<iterator*,iterator*> >::type
    get_children (Y const*) const {
      typedef iterator_T<void,R> iter_type;
      return std::make_pair(new iter_type(), new iter_type());
    }

    X const* x;
  };

  template <typename String, typename R>
  struct Query {
    Query (xpgtl::path<String> const& path, R const& r) {
      this->path = path;
      this->rds  = &r;
    }
  
    void go_to_next_valid () {
    }
  
    xpgtl::path<String> path;
    R const*            rds;
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

  for (++argv; argc > 0; --argc, ++argv) {
    std::cout << *argv << std::endl;
    try {
      std::wifstream wifstr(*argv);
      wifstr.imbue(loc);
      wifstr >> std::noskipws;
      std::istream_iterator<wchar_t,wchar_t> ctr(wifstr);
      std::istream_iterator<wchar_t,wchar_t> cnd;
      JSONpp::json_v json = JSONpp::parse(ctr, cnd);
      xpgtl::Query<std::string,JSONpp::json_v> Q(path, json);
      std::cout << std::endl;

    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
  }

  return 0;
}
