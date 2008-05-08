// xpgtl
#include "concepts.hpp"
// rdstl
#include <rdstl/rdstl.hpp>
// BEL
#include <bel/begin-end.hpp>
// boost
#include <boost/variant.hpp>
// STL
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#ifndef VPATH_LIB_BASICS
#define VPATH_LIB_BASICS

namespace xpgtl {

template <typename String>
struct self_valued_string : String {
  self_valued_string (String const& str=String()) : String(str) {}

  std::string as_string () const {
    return std::string(this->begin(), this->end());
  }
};

typedef boost::variant<
  rdstl::valued<std::size_t>,
  rdstl::valued<bool>
  > stl_reference_union;

template <typename C, typename X>
stl_reference_union
common_stl_attribute_selector (C const& c, std::string const& attr, X) {
  stl_reference_union sru;
  if ("size" == attr) {
    std::size_t size = c.size();
    sru = &size;
  } else if ("empty" == attr) {
    bool empty = c.empty();
    sru = &empty;
  }
  return sru;
}

// list
template <typename T, typename A, typename X>
std::string tag (std::list<T,A> const& v, xpath<X> x) {
  return "list";
}
template <typename T, typename A, typename X>
struct has_attributes<std::list<T,A>,X> : boost::mpl::true_ {};
template <typename T, typename A, typename X>
stl_reference_union
attribute (std::list<T,A> const& v, std::string const& attr, X x) {
  return common_stl_attribute_selector(v,attr,x);
}
// map
template <typename K, typename V, typename C, typename A, typename X>
std::string tag (std::map<K,V,C,A> const& v, xpath<X> x) {
  return "map";
}
template <typename K, typename V, typename C, typename A, typename X>
struct has_attributes<std::map<K,V,C,A>,X> : boost::mpl::true_ {};
template <typename K, typename V, typename C, typename A, typename X>
stl_reference_union
attribute (std::map<K,V,C,A> const& v, std::string const& attr, X x) {
  return common_stl_attribute_selector(v,attr,x);
}
// set
template <typename V, typename C, typename A, typename X>
std::string tag (std::set<V,C,A> const& v, xpath<X> x) {
  return "set";
}
template <typename V, typename C, typename A, typename X>
struct has_attributes<std::set<V,C,A>,X> : boost::mpl::true_ {};
template <typename V, typename C, typename A, typename X>
stl_reference_union
attribute (std::set<V,C,A> const& v, std::string const& attr, X x) {
  return common_stl_attribute_selector(v,attr,x);
}
// string
template <typename T, typename X>
std::string tag (std::basic_string<T> const& str, xpath<X> x) {
  return "string";
}
template <typename S, typename X>
std::string tag (self_valued_string<S> const& str, xpath<X> x) {
  return str.as_string();
}
// vector
template <typename T, typename A, typename X>
std::string tag (std::vector<T,A> const& v, xpath<X> x) {
  return "vector";
}
template <typename T, typename A, typename X>
struct has_attributes<std::vector<T,A>,X> : boost::mpl::true_ {};
template <typename T, typename A, typename X>
stl_reference_union
attribute (std::vector<T,A> const& v, std::string const& attr, X x) {
  return common_stl_attribute_selector(v,attr,x);
}

/*

There are two ways to deal with associative containers:
  1. create an iterator facade and just return the "value" from the
      key/value pair
  2. something ... more complicated

*/

namespace assoc_ctr {

template <typename AssociativeContainer>
struct value_iterator_facade {
  typedef value_iterator_facade<
              AssociativeContainer>         self_type;

  typedef AssociativeContainer              ac_type;
  typedef typename ac_type::const_iterator  const_iterator;
  typedef const_iterator                    iterator;

  typedef typename ac_type::key_type        ac_key_type;
  typedef typename ac_type::mapped_type     ac_mapped_type;
  typedef typename ac_type::value_type      ac_value_type;

  typedef ac_mapped_type                    value_type;
  typedef value_type const&                 reference;
  typedef value_type const*                 pointer;

  value_iterator_facade () {}
  value_iterator_facade (iterator const& itr)
    : iterator_(itr) {}
  value_iterator_facade (self_type const& st)
    : iterator_(st.iterator_) {}
  self_type& operator = (self_type const& st) {
    this->iterator_ = st.iterator_;
    return *this;
  }

  self_type& operator ++ () {
    ++this->iterator_;
    return *this;
  }
  self_type operator ++ (int) {
    self_type cp(*this);
    ++(*this);
    return cp;
  }
  value_type const& operator * () const {
    return this->iterator_->second;
  }
  value_type const* operator -> () const {
    return &this->iterator_->second;
  }

  friend bool operator == (self_type const& L, self_type const& R) {
    return L.iterator_ == R.iterator_;
  }
  friend bool operator != (self_type const& L, self_type const& R) {
    return L.iterator_ != R.iterator_;
  }

  iterator iterator_;
};

  } // end assoc_ctr namespace
} // end xpgtl namespace

namespace bel {

// list specialization
template <typename T, typename A, typename X>
struct iterator<std::list<T,A>, xpgtl::xpath<X> > {
  typedef typename std::list<T,A>::const_iterator type;
};
template <typename T, typename A, typename X>
typename iterator<std::list<T,A>, xpgtl::xpath<X> >::type
begin (std::list<T,A> const& t, xpgtl::xpath<X>) {
  return t.begin();
}
template <typename T, typename A, typename X>
typename iterator<std::list<T,A>, xpgtl::xpath<X> >::type
end (std::list<T,A> const& t, xpgtl::xpath<X>) {
  return t.end();
}
// map specialization
template <typename K, typename V, typename C, typename A, typename X>
struct iterator<std::map<K,V,C,A>, xpgtl::xpath<X> > {
  typedef xpgtl::assoc_ctr::value_iterator_facade<std::map<K,V,C,A> > type;
};
template <typename K, typename V, typename C, typename A, typename X>
typename iterator<std::map<K,V,C,A>, xpgtl::xpath<X> >::type
begin (std::map<K,V,C,A> const& t, xpgtl::xpath<X>) {
  typedef typename iterator<std::map<K,V,C,A>, xpgtl::xpath<X> >::type iter;
  return iter(t.begin());
}
template <typename K, typename V, typename C, typename A, typename X>
typename iterator<std::map<K,V,C,A>, xpgtl::xpath<X> >::type
end (std::map<K,V,C,A> const& t, xpgtl::xpath<X>) {
  typedef typename iterator<std::map<K,V,C,A>, xpgtl::xpath<X> >::type iter;
  return iter(t.end());
}
// set specialization
template <typename V, typename C, typename A, typename X>
struct iterator<std::set<V,C,A>, xpgtl::xpath<X> > {
  typedef typename std::set<V,C,A>::const_iterator type;
};
template <typename V, typename C, typename A, typename X>
typename iterator<std::set<V,C,A>, xpgtl::xpath<X> >::type
begin (std::set<V,C,A> const& t, xpgtl::xpath<X>) {
  return t.begin();
}
template <typename V, typename C, typename A, typename X>
typename iterator<std::set<V,C,A>, xpgtl::xpath<X> >::type
end (std::set<V,C,A> const& t, xpgtl::xpath<X>) {
  return t.end();
}
// vector specialization
template <typename T, typename A, typename X>
struct iterator<std::vector<T,A>, xpgtl::xpath<X> > {
  typedef typename std::vector<T,A>::const_iterator type;
};
template <typename T, typename A, typename X>
typename iterator<std::vector<T,A>, xpgtl::xpath<X> >::type
begin (std::vector<T,A> const& t, xpgtl::xpath<X>) {
  return t.begin();
}
template <typename T, typename A, typename X>
typename iterator<std::vector<T,A>, xpgtl::xpath<X> >::type
end (std::vector<T,A> const& t, xpgtl::xpath<X>) {
  return t.end();
}

} // end bel namespace

namespace rdstl {

// STL recursive types
template <typename T, typename A, typename X>
struct has_children<std::list<T,A>,X> : boost::mpl::true_ {};
template <typename T, typename A, typename X>
struct reference_union<std::list<T,A>,X> {
  typedef typename bel::iterator<std::list<T,A>, xpgtl::xpath<X> >::type type;
};
template <typename K, typename V, typename C, typename A, typename X>
struct has_children<std::map<K,V,C,A>,X> : boost::mpl::true_ {};
template <typename K, typename V, typename C, typename A, typename X>
struct reference_union<std::map<K,V,C,A>,X> {
  typedef typename bel::iterator<std::map<K,V,C,A>, xpgtl::xpath<X> >::type type;
};
template <typename K, typename C, typename A, typename X>
struct has_children<std::set<K,C,A>,X> : boost::mpl::true_ {};
template <typename K, typename C, typename A, typename X>
struct reference_union<std::set<K,C,A>,X> {
  typedef typename bel::iterator<std::set<K,C,A>, xpgtl::xpath<X> >::type type;
};
template <typename T, typename A, typename X>
struct has_children<std::vector<T,A>,X> : boost::mpl::true_ {};
template <typename T, typename A, typename X>
struct reference_union<std::vector<T,A>,X> {
  typedef typename bel::iterator<std::vector<T,A>, xpgtl::xpath<X> >::type type;
};

}// end rdstl namespace

#endif//VPATH_LIB_BASICS