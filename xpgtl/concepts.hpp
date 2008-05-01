#include <bel/begin-end.hpp>
#include <boost/variant.hpp>
#include <boost/mpl/bool.hpp>

#ifndef XPGTL_LIB_XPGTL
#define XPGTL_LIB_XPGTL

// TODO:
//  * port to use RDSTL

namespace xpgtl {

// xpath-tag; the "X" is the global data-structure
// type which we are recursing over
// the function "xpath_tag" builds a tag for us from a pointer
template <typename X=void> struct xpath {};
template <typename X>
xpath<X> xpath_tag (X *gds=0) { return xpath<X>(); }

// to simplify things later...
template <typename POD, typename X>
struct builtin_tag_base : boost::mpl::true_ { typedef std::string type; };

// attributes metafunction; if "true" then the type T
// has attributes, the discriminated union type that
// stores references (pointer-like objects) to the attributes
// are located at: has_attributes<T,Tag>::type
template <typename T, typename Tag>
struct has_attributes : boost::mpl::false_ {};

}  // end xpgtl namespace

#endif//XPGTL_LIB_XPGTL
