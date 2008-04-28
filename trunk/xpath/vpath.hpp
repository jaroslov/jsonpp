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

// the default "tag" function, which returns a string-name
// for a given type
template <typename T, typename GDS>
std::string tag (T, xpath<GDS>) { return "no-name"; }

// recursive metafunction; if "true" then the type T is
// recursive and supports an iterator over the children,
// see "children"
template <typename T>
struct recursive : boost::mpl::false_ {};

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

// the children-union metafunction determines the type
// of the discriminated union which contains the set of
// children types; also the element type of the result
// set for any query
//
// this currently defaults to being the value-type of the
// children-iterator, but normally it is the type "X"
template <typename T, typename X>
struct children_union {
  typedef typename bel::iterator<T, xpath<X> >::type::value_type type;
};

struct attribute_inner_quit {};

// metafunction tells us the discriminated-union
// type for the set of attribute types of the type T
template <typename T, typename X> struct attribute_union {
  /*
    We can't even begin to *guess* at this...
    So here's what we do:
      1. we set the inner-type here to be "attribute_inner_quit",
          this "turns off" attributes
      2. whenever we see a attribute:: axis, we make a call
          to the "handle_attribute" function, and we
          use enable/disable -_if: if the type here is "void"
          we have an empty function, otherwise, it is non-empty,
          i.e., does stuff
  */
  typedef attribute_inner_quit type;
  static const signed char kill_length = -1;
};

// retrieves a given attribute "by name", as in, a string
template <typename T, typename X>
typename attribute_union<T,xpath<X> >::type
attribute (T const& t, std::string const& str, xpath<X> x) {
  const char ASSERT_DEFAULT_ATTRIBUTE_ALWAYS_FAILS[attribute_union<T,xpath<X> >::kill_length];
}

}  // end vpath namespace

#endif//VPATH_LIB_VPATH
