// local stuff
#include "builtin.hpp"
#include "path.hpp"
#include "vpath.hpp"
// boost stuff
#include <boost/tuple/tuple.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>
// STL stuff
#include <stdexcept>

#ifndef VPATH_LIB_QUERY
#define VPATH_LIB_QUERY

namespace vpath {

template <typename String, typename X>
struct query_generator {
  typedef path::path_type<String>           path_t;
  typedef xpath<X>                          xpath_t;
  typedef vpath::children_union<X,xpath_t>  cu_mf;
  typedef typename cu_mf::type              c_union_t;
  typedef std::vector<c_union_t>            result_type;
  typedef query_generator<String,X>         qg_type;

  /*
    1. we need to pass the path & current position in the path along
    2. we need to store the "parent" (both visitor & node) in the
        visitor, so we can find the parent, i.e., building a runtime
        dynamic tree
    3. we need to implement the path stuff... =)
  */

  template <typename Node, typename Parent=void>
  struct visitor {
    typedef visitor<Node,Parent>          self_type;
    typedef path_t                        path_type;
    typedef Node                          node_type;
    typedef typename qg_type::result_type result_type;

    std::string indent;

    visitor (node_type const* parent=0,
      path_type const* path=0, std::size_t axis=0)
      : parent(parent), path(path), axis(axis) {}

    template <typename T>
    typename boost::enable_if<
        boost::is_same<vpath::attribute_inner_quit,
          typename vpath::attribute_union<T,xpath_t>::type>
      >::type
    handle_attribute (T const& t) const {
      std::cout << this->indent << "No attributes" << std::endl;
    }

    template <typename T>
    typename boost::disable_if<
        boost::is_same<vpath::attribute_inner_quit,
          typename vpath::attribute_union<T,xpath_t>::type>
      >::type
    handle_attribute (T const& t) const {
      std::cout << this->indent << "Has attributes" << std::endl;
    }

    template <typename T>
    typename boost::disable_if<recursive<T>,result_type>::type
    operator () (T const& t) const {
      // non-recursive
      std::cout << this->indent << "Terminal Tag: " << tag(t, xpath_t()) << std::endl;
      return result_type();
    }

    template <typename T>
    bool apply_predicate (T const& t) const {
      // ignore the predicate for now
      return true;
    }

    template <typename T>
    typename boost::enable_if<recursive<T>,result_type>::type
    operator () (T const& t) const {
      // recursive
      typedef typename bel::iterator<T,xpath_t>::type iterator;
      if (this->path->size() <= this->axis)
        return result_type();
      result_type result_set;
      std::cout << this->indent << "Recursive Tag: " << tag(t, xpath_t()) << std::endl;
      visitor<T,self_type> V(&t, this->path);
      V.indent = this->indent+"  ";
      iterator first, last;
      boost::tie(first,last) = vpath::children(t, xpath_t());
      switch ((*this->path)[this->axis].name) {
      case axis_t::ancestor : {
          throw std::runtime_error("Unsupported axis-name: ancestor");
        } break;
      case axis_t::ancestor_or_self : {
          throw std::runtime_error("Unsupported axis-name: ancestor-or-self");
        } break;
      case axis_t::attribute : {
          this->handle_attribute(t);
        } break;
      case axis_t::child : {
          V.axis = this->axis+1;
          if (tag(t, xpath_t()) == this->path->test(this->axis))
            for ( ; first != last; ++first)
              if (this->apply_predicate(t)) {
                result_type subrs = visit(V, *first);
                result_set.insert(bel::end(result_set),
                              bel::begin(subrs), bel::end(subrs));
              }
        } break;
      case axis_t::descendent : {
          throw std::runtime_error("Unsupported axis-name: descendent");
        } break;
      case axis_t::descendent_or_self : {
          throw std::runtime_error("Unsupported axis-name: descendent-or-self");
        } break;
      case axis_t::following: {
          throw std::runtime_error("Unsupported axis-name: following");
        } break;
      case axis_t::following_sibling: {
          throw std::runtime_error("Unsupported axis-name: following-sibling");
        } break;
      case axis_t::namespace_: {
          throw std::runtime_error("Unsupported axis-name: namespace");
        } break;
      case axis_t::parent: {
          throw std::runtime_error("Unsupported axis-name: parent");
        } break;
      case axis_t::preceding: {
          throw std::runtime_error("Unsupported axis-name: preceding");
        } break;
      case axis_t::preceding_sibling: {
          throw std::runtime_error("Unsupported axis-name: preceding-sibling");
        } break;
      case axis_t::self: {
          throw std::runtime_error("Unsupported axis-name: self");
        } break;
      default: {
          std::cout << "Unsupported axis-name." << std::endl;
        }
      }
      return result_set;
    }

    Parent const*     predecessor;
    node_type const*  parent;
    path_type const*  path;
    std::size_t       axis;
  };

  result_type operator () (path_t const& path, X const& gds) {
    visitor<X> V(&gds,&path);;
    V.predecessor = 0;
    return visit(V, gds);
  }
};

template <typename X, typename String>
void query (path::path_type<String> const& path, X const& gds) {
  query_generator<String,X> qg;
  qg(path, gds);
}

template <typename X, typename String>
void query (String const& path, X const& gds) {
  query(path::path_type<String>(path), gds);
}

} // end vpath

#endif//VPATH_LIB_QUERY
