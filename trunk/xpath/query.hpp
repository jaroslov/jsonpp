#include "builtin.hpp"
#include "path.hpp"
#include "vpath.hpp"
#include <boost/tuple/tuple.hpp>
#include <boost/utility/enable_if.hpp>
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

  template <typename Node>
  struct visitor {
    typedef path_t  path_type;
    typedef Node    node_type;
    typedef void    result_type; // visitor concept

    std::string indent;

    visitor (node_type const* parent=0,
      path_type const* path=0, std::size_t axis=0)
      : parent(parent), path(path), axis(axis) {}

    template <typename T>
    typename boost::disable_if<recursive<T> >::type
    operator () (T const& t) const {
      // non-recursive
      std::cout << this->indent
        << "Terminal Tag: " << tag(t, xpath_t()) << std::endl;
    }

    template <typename T>
    typename boost::enable_if<recursive<T> >::type
    operator () (T const& t) const {
      // recursive
      typedef typename bel::iterator<T,xpath_t>::type iterator;
      if (this->path->size() <= this->axis)
        return;
      std::cout << this->indent
        << "Recursive Tag: " << tag(t, xpath_t()) << std::endl;
      visitor<T> V(&t, this->path);
      V.indent = this->indent+"  ";
      iterator first, last;
      boost::tie(first,last) = bel::sequence(t, xpath_t());
      switch ((*this->path)[this->axis].name) {
      case axis_t::child : {
          /// the current test case is with JSON, and right now, the
          // "map" and "vector" etc. data-structures still return
          // the xpath::no_iterator iterator
          V.axis = this->axis+1;
          if (tag(t, xpath_t()) == this->path->test(this->axis))
            for ( ; first != last; ++first)
              visit(V, *first);
        } break;
      default: {
          std::cout << "Unsupported axis-name." << std::endl;
        }
      }
    }

    node_type const*  parent;
    path_type const*  path;
    std::size_t       axis;
  };

  void operator () (path_t const& path, GlobalDS const& gds) {
    visitor<GlobalDS> V(&gds,&path);;
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
