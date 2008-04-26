#include "path.hpp"
#include <stdexcept>

#ifndef VPATH_LIB_QUERY
#define VPATH_LIB_QUERY

namespace vpath { namespace query {

/*

  Definition:

  Discriminated Union
  
  template <typename T, typename U>
  U* u = cast(T, U*);
  if 0 == u:
    T does not store a "U"
  else
    T stores a "U"

  X_1, X_2, ..., X_N -- types
  x_i : X_i -- x_i is a value of X_i
  t : T -- t is a discriminated union
  
  t = x_i;
  if legal, then return type is t
  else, throw std::illegal_assignment();

*/

struct illegal_assignment : public std::exception {
  virtual ~ illegal_assignment () throw() {}
  virtual const char* what () const throw() {
    return "Illegal assignment to discriminated union.";
  }
};

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

template <typename String, typename GlobalDS>
struct query_generator {
  typedef path::path_type<String> path_t;
  void operator () (path_t const& path, GlobalDS const& gds) {
  }
};

template <typename GlobalDS, typename String>
void query (path::path_type<String> const& path, GlobalDS const& gds) {
  query_generator<GlobalDS> qg;
  qg(path, gds);
}

template <typename GlobalDS, typename String>
void query (String const& path, GlobalDS const& gds) {
  path::path_type<String> path_t(path);
  query_generator<GlobalDS> qg;
  qg(path_t, gds);
}

  } // end query
} // end vpath

#endif//VPATH_LIB_QUERY
