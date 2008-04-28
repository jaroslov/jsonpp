#include <boost/variant.hpp>
#include <boost/mpl/bool.hpp>

#ifndef VPATH_LIB_VPATH
#define VPATH_LIB_VPATH

namespace vpath {

// xpath-tag; the "GlobalDS" is the global data-structure
// type which we are recursing over
// the function "xpath_tag" builds a tag for us from a pointer
template <typename GlobalDS=void> struct xpath {};
template <typename GlobalDS>
xpath<GlobalDS> xpath_tag (GlobalDS *gds=0) { return xpath<GlobalDS>(); }

template <typename T, typename GlobalDS>
struct attribute_ {
  typedef GlobalDS type; // we assume the global-type is a variant
    // and can store whatever we need
};

template <typename T, typename GlobalDS>
typename attribute_<T,xpath<GlobalDS> >::type const&
attribute (T const& t, std::string const& str, xpath<GlobalDS>) {
  return t.attribute(str);
}

template <typename Visitor, typename Variant>
typename Visitor::result_type
visit (Visitor const& visitor, Variant const& variant) {
  // we assume that the basic variant is a boost::variant
  // this only works on two conditions:
  //  1. variant really is a variant
  //  2. someone else `includes' variant
  return boost::apply_visitor(visitor, variant);
}

template <typename T, typename GDS>
std::string tag (T, xpath<GDS>) { return "T"; }

template <typename T>
struct recursive : boost::mpl::false_ {};

}  // end vpath namespace

#endif//VPATH_LIB_VPATH
