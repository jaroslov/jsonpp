#include "jsonpp.hpp"
#include <xpgtl/xpath.hpp>
#include <rdstl/rdstl.hpp>

#ifndef JSONPP_XPGTL_INTERFACE
#define JSONPP_XPGTL_INTERFACE

namespace JSONpp {

  // given an iterator from an object we have two values
  //      <string,json>
  // we want to build an interface like this:
  //      self::object/foo/vector
  // where "foo" is the key (string) in the iterator
  //
  // Requirements:
  //  1. children(object) -> iterator
  //  2. *iterator -> variant
  //  3. tag(visit(variant)) == iterator->first
  //  4. children(visit(variant)) -> trivial-iterator
  //  5. *trivial-iterator -> iterator-second
  //
  // So... we have to wrap the std::map<...>::const_iterator
  // so that on dereference it returns a variant<T>
  // where T is an adaptor storing a std::map<...>::const_iterator (from above)
  // and tag(adaptor) returns the value of the entry as a std::string
  // and children(adaptor) returns a trivial-iterator of the second

  // Entry Iterator
  template <typename AssociativeContainer>
  struct entry {
    typedef AssociativeContainer            ac_t;
    typedef typename ac_t::const_iterator   ac_iter_t;
    typedef typename ac_iter_t::value_type  ac_val_t;
    typedef typename ac_val_t::second_type  value_type;
    typedef entry<ac_t>                     self_type;
    typedef self_type                       iterator;

    entry (ac_iter_t const& acit=ac_iter_t(), bool ae=false)
      : at_end(ae), iterator_(acit) {}

    std::string tag () const { // tag support
      return JSONpp::to_ascii_string(this->iterator_->first);
    }
    // support for begin/end library
    self_type begin () const { return self_type(this->iterator_,false); }
    self_type end () const { return self_type(this->iterator_,true); }
    void set_iterator (ac_iter_t const& acit) {
      this->iterator_ = acit;
    }

    // forward iterator interface
    value_type const& operator * () const {
      return this->iterator_->second;
    }
    value_type const* operator -> () const {
      return &this->iterator_->second;
    }
    self_type& operator ++ () {
      this->at_end = true;
      return *this;
    }
    self_type operator ++ (int) {
      self_type cp(*this); ++*this;
      return cp;
    }
    friend bool operator == (self_type const& L, self_type const& R) {
      return (L.at_end == R.at_end) and (L.iterator_ == R.iterator_);
    }
    friend bool operator != (self_type const& L, self_type const& R) {
      return (L.at_end != R.at_end) or (L.iterator_ != R.iterator_);
    }
    friend bool operator < (self_type const& L, self_type const& R) {
      return L.iterator_->first < R.iterator_->first;
    }

    template <typename CharT>
    friend std::basic_ostream<CharT>&
    operator << (std::basic_ostream<CharT>& bostr, self_type const& st) {
      bostr << "Some Entry";
      return bostr;
    }

    bool      at_end;
    ac_iter_t iterator_;
  };

  // object-entry iterator facade
  template <typename AssociativeContainer>
  struct object_entry_iterator {
    typedef AssociativeContainer            ac_t;
    typedef typename ac_t::const_iterator   ac_iter_t;
    typedef object_entry_iterator<ac_t>     self_type;
    typedef boost::variant<entry<ac_t> >    value_type;

    object_entry_iterator (ac_iter_t const& iter=ac_iter_t())
      : iterator(iter), proxy() {
      this->proxy = entry<ac_t>(this->iterator, false);
    }
    object_entry_iterator (self_type const& st) {
      this->iterator = st.iterator;
      this->proxy = st.proxy;
    }
    self_type& operator = (self_type const& st) {
      this->iterator = st.iterator;
      this->proxy = st.proxy;
      return *this;
    }

    // forward iterator interface
    value_type const& operator * () const {
      return this->proxy;
    }
    value_type const* operator -> () const {
      return &this->proxy;
    }
    self_type& operator ++ () {
      ++this->iterator;
      entry<ac_t> entry(this->iterator, false);
      this->proxy = entry;
      return *this;
    }
    self_type operator ++ (int) {
      self_type cp(*this); ++*this;
      return cp;
    }
    friend bool operator == (self_type const& L, self_type const& R) {
      return L.iterator == R.iterator;
    }
    friend bool operator != (self_type const& L, self_type const& R) {
      return L.iterator != R.iterator;
    }

    ac_iter_t   iterator;
    value_type  proxy;
  };

} // end json namespace

namespace rdstl {
  template <typename X> struct reference_union<JSONpp::json_v, xpgtl::xpath<X> > {
    typedef boost::variant<
      typename JSONpp::json_gen::value_t const*,
      typename JSONpp::json_gen::string_t const*,
      typename JSONpp::json_gen::number_t const*,
      typename JSONpp::json_gen::object_t const*,
      typename JSONpp::json_gen::array_t const*,
      typename JSONpp::json_gen::bool_t const*,
      typename JSONpp::json_gen::null_t const*,
      JSONpp::entry<typename JSONpp::json_gen::object_t> const*,
      rdstl::valued<std::size_t>
      > type;
  };

  template <typename X>
  struct has_children<std::map<std::wstring,JSONpp::json_v>, xpgtl::xpath<X> > : boost::mpl::true_ {};
  template <typename X>
  struct reference_union<std::map<std::wstring,JSONpp::json_v>, xpgtl::xpath<X> > {
    typedef JSONpp::object_entry_iterator<std::map<std::wstring,JSONpp::json_v> > type;
  };

  template <typename X>
  struct is_proxy<JSONpp::entry<std::map<std::wstring,JSONpp::json_v> >, xpgtl::xpath<X> >
    : boost::mpl::true_ {};

  template <typename X>
  struct has_children<JSONpp::entry<std::map<std::wstring,JSONpp::json_v> >, xpgtl::xpath<X> > : boost::mpl::true_ {};
  template <typename X>
  struct reference_union<JSONpp::entry<std::map<std::wstring,JSONpp::json_v> >, xpgtl::xpath<X> > {
    typedef typename
      JSONpp::entry<std::map<std::wstring,JSONpp::json_v> >::iterator type;
  };
} // end rdstl namespace

namespace bel {

// map specialization
template <typename X>
struct iterator<std::map<std::wstring,JSONpp::json_v>, xpgtl::xpath<X> > {
  typedef JSONpp::object_entry_iterator<std::map<std::wstring,JSONpp::json_v> > type;
};
template <typename X>
typename iterator<std::map<std::wstring,JSONpp::json_v>, xpgtl::xpath<X> >::type
begin (std::map<std::wstring,JSONpp::json_v> const& t, xpgtl::xpath<X>) {
  typedef typename
    iterator<std::map<std::wstring,JSONpp::json_v>,
      xpgtl::xpath<X> >::type iter;
  return iter(t.begin());
}
template <typename X>
typename iterator<std::map<std::wstring,JSONpp::json_v>, xpgtl::xpath<X> >::type
end (std::map<std::wstring,JSONpp::json_v> const& t, xpgtl::xpath<X>) {
  typedef typename
    iterator<std::map<std::wstring,JSONpp::json_v>,
      xpgtl::xpath<X> >::type iter;
  return iter(t.end());
}
// entry specialization
template <typename X>
struct iterator<JSONpp::entry<std::map<std::wstring,JSONpp::json_v> >, xpgtl::xpath<X> > {
  typedef typename JSONpp::entry<std::map<std::wstring,JSONpp::json_v> >::iterator type;
};
template <typename X>
typename iterator<std::map<std::wstring,JSONpp::json_v>, xpgtl::xpath<X> >::type
begin (JSONpp::entry<std::map<std::wstring,JSONpp::json_v> > const& t, xpgtl::xpath<X>) {
  return t.begin();
}
template <typename X>
typename iterator<std::map<std::wstring,JSONpp::json_v>, xpgtl::xpath<X> >::type
end (JSONpp::entry<std::map<std::wstring,JSONpp::json_v> > const& t, xpgtl::xpath<X>) {
  return t.end();
}

}// end bel namespace

//
// augment the "tag" overload set for nil and json_v
namespace JSONpp {
  std::string tag (JSONpp::nil, xpgtl::xpath<JSONpp::json_v>) {
    return "nil";
  }
  std::string tag (bool, xpgtl::xpath<JSONpp::json_v>) {
    return "bool";
  }
  std::string tag (std::wstring, xpgtl::xpath<JSONpp::json_v>) {
    return "string";
  }
  std::string tag (std::vector<JSONpp::json_v>, xpgtl::xpath<JSONpp::json_v>) {
    return "array";
  }
  std::string tag (entry<std::map<std::wstring,JSONpp::json_v> > const& e, xpgtl::xpath<JSONpp::json_v>) {
    return e.tag();
  }
  std::string tag (std::map<std::wstring,JSONpp::json_v>,xpgtl::xpath<JSONpp::json_v>) {
    return "object";
  }
  std::string tag (JSONpp::json_v, xpgtl::xpath<JSONpp::json_v>) {
    return "json";
  }
} // end json namespace

#endif//JSONPP_XPGTL_INTERFACE
