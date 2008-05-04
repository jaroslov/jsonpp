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
// has attributes; the value of the attributes must fit
// within the reference_union of T
template <typename T, typename Tag>
struct has_attributes : boost::mpl::false_ {};

// this is here just to demonstrate *what* the attribute-acquisition
// function looks like
struct no_attr {};
template <typename T, typename Tag>
no_attr attribute (T, std::string, Tag) {
  return no_attr();
}

}  // end xpgtl namespace

#endif//XPGTL_LIB_XPGTL
