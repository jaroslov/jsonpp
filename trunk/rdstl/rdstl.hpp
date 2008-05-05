
#include <bel/begin-end.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/variant.hpp>

#ifndef RDST_LIBRARY
#define RDST_LIBRARY

namespace rdstl {

// tag metafunction; if "true" then the type T
// has a tag-name, which is some Regular type which
// is located at: has_tag<T,Tag>::type
template <typename T, typename Tag>
struct has_tag : boost::mpl::false_ {};
// the related "tag" function returns the tag for
// a given type T given the parent data-structure P,
// for some tag-type Tag
template <typename P, typename T, typename Tag>
typename has_tag<T,Tag>::type tag (P, T, Tag) {
  return typename has_tag<T,Tag>::type();
}
template <typename T, typename Tag>
typename has_tag<T,Tag>::type tag (T, Tag) {
  return typename has_tag<T,Tag>::type();
}

// parent metafunction; if "true" then the type T
// knows its parent, the discriminated union type that
// stores references (pointer-like objects) to the parent
// is located at: knows_parent<T,Tag>::type
template <typename T, typename Tag>
struct knows_parent : boost::mpl::false_ {};
// the related "parent" function returns the parent of
// a given value t:T given the parent data-structure P,
// for some tag-type Tag
template <typename P, typename T, typename Tag>
typename knows_parent<T,Tag>::type const&
parent (P, T const& t, Tag) {
  return t.parent();
}

// children metafunction; if "true" then the type T is
// recursive and supports an iterator over the children,
// see "children"
template <typename T, typename Tag>
struct has_children : boost::mpl::false_ {};
// the references traits; a discriminated union type for
// T and Tag that can hold a reference to any element
// in the RDS; i.e., if U is the reference, then
// for all x in X, for X the types in the RDS, then
// U = &x;
// The reference-union exists if the has_children function
// is "true"
template <typename T, typename Tag>
struct reference_union {
  typedef typename T::reference_union type;
};
// the "children" retrieves an iterator to the first
// and last elements of a sequence over the children of
// the type T; it defaults to calling the begin-end library's
// "sequence" function
template <typename T, typename Tag>
std::pair<typename bel::iterator<T,Tag>::type,
  typename bel::iterator<T,Tag>::type>
children (T const& t, Tag x) {
  return bel::sequence(t, x);
}

// proxy metafunction; if "true" then the type T is
// a proxy, and should not be stored
template <typename T, typename Tag>
struct is_proxy : boost::mpl::false_ {};

// the default "visit" function, this should be overloaded if
// you use a different discriminated union kind
template <typename Visitor, typename Variant>
typename Visitor::result_type
visit (Visitor const& visitor, Variant const& variant) {
  // we assume that the basic variant is a boost::variant
  // this only works on two conditions:
  //  1. variant really is a variant
  //  2. someone else `includes' variant
  return boost::apply_visitor(visitor, variant);
}

// the value metafunction and traits class; whether or
// not an RDS has a value-union, and the type thereof
template <typename T, typename Tag>
struct has_value_union : boost::mpl::false_ {};
template <typename T, typename Tag>
struct value_union {
  typedef typename T::reference_union type;
};

// takes a reference (T const*) to T, and stores the value
// of T
template <typename T>
struct valued {
  valued () : value() {}
  explicit valued (T const& t) : value(t) {}
  valued (const T* t) : value() {
    if (0 != t) this->value = *t;
  }
  valued (valued<T> const& V) : value(V.value) {}
  valued<T>& operator = (valued<T> const& V) {
    this->value = V.value; return *this;
  }
  valued<T>& operator = (T const& t) {
    this->value = t; return *this;
  }
  valued<T>& operator = (const T* t) {
    if (0 != t) this->value = *t;
    return *this;
  }
  const T* operator -> () const {
    return &this->value;
  }
  T const& operator * () const {
    return this->value;
  }
  const T* get () const {
    return &this->value;
  }

  friend bool operator < (valued<T> const& L, valued<T> const& R) {
    return L.get() < R.get();
  }

  template <typename Char>
  friend std::basic_ostream<Char>&
  operator << (std::basic_ostream<Char>& bostr, valued<T> const& V) {
    bostr << V.get();
    return bostr;
  }

  T value;
};

}

#endif//RDST_LIBRARY
