#include "path.hpp"
#include "vpath.hpp"
#include <stdexcept>

#ifndef VPATH_LIB_QUERY
#define VPATH_LIB_QUERY

namespace vpath {

template <typename String, typename GlobalDS>
struct query_generator {
  typedef path::path_type<String> path_t;
  typedef xpath<GlobalDS>         xpath_t;

  struct visitor {
    typedef void result_type; // visitor concept
    template <typename T>
    void operator () (T const& t) const {
      typedef typename bel::iterator<T,xpath_t>::type iterator;
    }
  };

  void operator () (path_t const& path, GlobalDS const& gds) {
    visitor V;
    visit(V, gds);
  }
};

template <typename GlobalDS, typename String>
void query (path::path_type<String> const& path, GlobalDS const& gds) {
  query_generator<String,GlobalDS> qg;
  qg(path, gds);
}

template <typename GlobalDS, typename String>
void query (String const& path, GlobalDS const& gds) {
  query(path::path_type<String>(path), gds);
}

} // end vpath

#endif//VPATH_LIB_QUERY
