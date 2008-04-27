#include "builtin.hpp"
#include "path.hpp"
#include "vpath.hpp"
#include <boost/tuple/tuple.hpp>
#include <stdexcept>

#ifndef VPATH_LIB_QUERY
#define VPATH_LIB_QUERY

namespace vpath {

template <typename String, typename GlobalDS>
struct query_generator {
  typedef path::path_type<String> path_t;
  typedef xpath<GlobalDS>         xpath_t;

  /*
    1. we need to pass the path & current position in the path along
    2. we need to store the "parent" (both visitor & node) in the
        visitor, so we can find the parent, i.e., building a runtime
        dynamic tree
    3. we need to implement the path stuff... =)
  */

  struct visitor {
    typedef void result_type; // visitor concept
    template <typename T>
    void operator () (T const& t) const {
      typedef typename bel::iterator<T,xpath_t>::type iterator;
      iterator itr, ind;
      for (boost::tie(itr,ind)=bel::sequence(t, xpath_t()); itr!=ind; ++itr)
        ;
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
