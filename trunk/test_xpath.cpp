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
    virtual void next () = 0;
    virtual void prev () = 0;
    virtual bool eq (abstract_iterator const*) = 0;
    virtual bool n_eq (abstract_iterator const*) = 0;
    virtual abstract_visitor* dereference () const = 0;
    virtual abstract_iterator* clone () const = 0;
  };
  struct abstract_visitor {
    virtual ~ abstract_visitor () {}
    virtual std::pair<abstract_iterator*,abstract_iterator*>
      get_children () const = 0;
    virtual std::string get_tag () const = 0;
    virtual bool get_proxy () const = 0;
    virtual abstract_visitor* get_parent () const = 0;
    virtual abstract_visitor* get_attribute (std::string const&) const = 0;
    virtual abstract_visitor* clone () const = 0;
  };

  template <typename X, typename Tag> struct make_abstract_visitor;
  template <typename X, typename Tag> struct abstract_iterator_T;
  template <typename X, typename Tag> struct abstract_visitor_T;

  template <typename X, typename Tag>
  struct abstract_iterator_T : public abstract_iterator {
    X x;
    abstract_iterator_T (X const& x=X()) : x(x) {}
    virtual ~ abstract_iterator_T () {}
    virtual void next () { ++this->x; }
    virtual void prev () { ++this->x; }
    virtual bool eq (abstract_iterator const* it) {
      return this->x == static_cast<const abstract_iterator_T<X,Tag>*>(it)->x;
    }
    virtual bool n_eq (abstract_iterator const* it) {
      return this->x != static_cast<const abstract_iterator_T<X,Tag>*>(it)->x;
    }
    virtual abstract_visitor* dereference () const {
      make_abstract_visitor<X,Tag> M;
      return 0;
    }
    virtual abstract_iterator* clone () const {
      return new abstract_iterator_T<X,Tag>(this->x);
    }
  };

  template <typename Tag>
  struct abstract_iterator_T<void,Tag> : public abstract_iterator {
    template <typename X> abstract_iterator_T (X const& x) {}
    abstract_iterator_T () {}
    virtual ~ abstract_iterator_T () {}
    virtual void next () {}
    virtual void prev () {}
    virtual bool eq (abstract_iterator const*) { return false; }
    virtual bool n_eq (abstract_iterator const*) { return true; }
    virtual abstract_visitor* dereference () const { return 0; }
    virtual abstract_iterator* clone () const {
      return new abstract_iterator_T<void,Tag>();
    }
  };

  template <typename X, typename Tag>
  struct abstract_visitor_T : public abstract_visitor {
    typedef Tag xpath_t;

    virtual ~ abstract_visitor_T () {}
    abstract_visitor_T (X const& x) : x(&x) {}
    virtual std::pair<abstract_iterator*,abstract_iterator*>
    get_children () const {
      return this->get_children(this->x);
    }
    virtual std::string get_tag () const {
      return tag(*this->x, xpath_t());
    }
    virtual bool get_proxy () const {
      return typename rdstl::is_proxy<X,xpath_t>::type().value;
    }
    virtual abstract_visitor* get_parent () const {
      return 0;
    }
    virtual abstract_visitor* get_attribute (std::string const&) const {
      return 0;
    }
    virtual abstract_visitor* clone () const {
      return new abstract_visitor_T<X,Tag>(*this->x);
    }

    template <typename Y>
    typename boost::enable_if<rdstl::has_children<Y,xpath_t>,
      std::pair<abstract_iterator*,abstract_iterator*> >::type
    get_children (Y const*) const {
      typedef typename bel::iterator<Y,xpath_t>::type ch_iter;
      typedef abstract_iterator_T<ch_iter,Tag> iter_type;
      ch_iter f, l;
      boost::tie(f,l) = rdstl::children(*this->x, xpath_t());
      return std::make_pair(new iter_type(f), new iter_type(l));
    }
    template <typename Y>
    typename boost::disable_if<rdstl::has_children<Y,xpath_t>,
      std::pair<abstract_iterator*,abstract_iterator*> >::type
    get_children (Y const*) const {
      typedef abstract_iterator_T<void,Tag> iter_type;
      return std::make_pair(new iter_type(), new iter_type());
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
      work_item ()
        : axis_index(0), proxy(false) {}
      work_item (work_item const& w) {
        this->visitor = w.visitor;
      }
      explicit work_item (abstract_visitor* av) {
        this->visitor = shared_av(av);
      }
      work_item& operator = (work_item const& w) {
        this->visitor = w.visitor;
        return *this;
      }
      virtual ~ work_item () {}

      shared_av visitor;
      bool proxy;
      std::size_t axis_index;
    };

    Query (xpgtl::path<String> const& path, X const& x) {
      this->path = path;
      this->x  = &x;
      this->setup();
    }

    virtual ~ Query () {
      this->work_stack.empty();
      this->x = 0;
    }

    void setup () {
      this->work_stack.clear();
      this->axis_index = 0;
      make_abstract_visitor<X,xpath_t> M;
      abstract_visitor *av = rdstl::visit(M, *this->x);
      work_item w(av);
      this->work_stack.push_back(w);
    }

    void go_to_next_valid () {
      while (not this->work_stack.empty()) {
        axis_t Axis = this->path[this->work_stack.back().axis_index];
        switch (Axis.name) {
        case axis_t::self: this->handle_self(Axis); break;
        default: throw std::runtime_error("Unknown axis-name!");
        }
      }
      std::cout << "Found?" << std::endl;
    }

    void handle_self (axis_t const& axis) {
      //if (this->work_stack.back().visitor->get_tag() == this->path.test(axis)) {
      //  ++this->axis_index;
      //}
      this->work_stack.pop_back();
    }

    std::size_t           axis_index;
    std::list<work_item>  work_stack;
    xpgtl::path<String>   path;
    X const*              x;
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
      Q.go_to_next_valid();
      std::cout << std::endl;
    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
  }
  return 0;
}
