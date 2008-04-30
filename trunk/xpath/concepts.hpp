#include <bel/begin-end.hpp>
#include <boost/variant.hpp>
#include <boost/mpl/bool.hpp>

#ifndef VPATH_LIB_VPATH
#define VPATH_LIB_VPATH

namespace vpath {

// xpath-tag; the "X" is the global data-structure
// type which we are recursing over
// the function "xpath_tag" builds a tag for us from a pointer
template <typename X=void> struct xpath {};
template <typename X>
xpath<X> xpath_tag (X *gds=0) { return xpath<X>(); }

// children metafunction; if "true" then the type T is
// recursive and supports an iterator over the children,
// see "children"
template <typename T, typename Tag>
struct has_children : boost::mpl::false_ {};

// attributes metafunction; if "true" then the type T
// has attributes, the discriminated union type that
// stores references (pointer-like objects) to the attributes
// are located at: has_attributes<T,Tag>::type
template <typename T, typename Tag>
struct has_attributes : boost::mpl::false_ {};

// parent metafunction; if "true" then the type T
// knows its parent, the discriminated union type that
// stores references (pointer-like objects) to the parent
// is located at: knows_parent<T,Tag>::type
template <typename T, typename Tag>
struct knows_parent : boost::mpl::false_ {};

// the default "tag" function, which returns a string-name
// for a given type
template <typename T, typename GDS>
std::string tag (T, xpath<GDS>) { return "no-name"; }

// the "children" retrieves an iterator to the first
// and last elements of a sequence over the children of
// the type T; it defaults to calling the begin-end library's
// "sequence" function
template <typename T, typename X>
std::pair<typename bel::iterator<T,xpath<X> >::type,
  typename bel::iterator<T,xpath<X> >::type>
children (T const& t, xpath<X> x) {
  return bel::sequence(t, x);
}

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

}  // end vpath namespace

#endif//VPATH_LIB_VPATH
