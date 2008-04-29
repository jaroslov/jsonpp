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
  typedef vpath::has_children<X,xpath_t>    cu_mf;
  typedef typename cu_mf::type              c_union_t;
  typedef std::vector<c_union_t>            result_type;
  typedef query_generator<String,X>         qg_type;

  struct visitor_base {
    virtual c_union_t const& node () const {
      return this->node_;
    }
    virtual c_union_t& node () {
      return this->node_;
    }
    c_union_t node_;
  };

  template <typename Node>
  struct visitor : public visitor_base {
    typedef visitor<Node>                 self_type;
    typedef path_t                        path_type;
    typedef Node                          node_type;
    typedef typename qg_type::result_type result_type;

    std::string indent () const {
      return std::string(2*this->axis,' ');
    }

    visitor (node_type const* node=0, path_type const* path=0,
      std::size_t axis=0, visitor_base const* parent=0)
      : path(path), axis(axis), parent(parent) {
      this->node() = node;
    }

    template <typename T>
    typename boost::disable_if<has_attributes<T,xpath_t>, result_type>::type
    handle_attribute (T const& t) const {
      std::cout << this->indent() << "No attributes" << std::endl;
    }
    template <typename T>
    typename boost::enable_if<has_attributes<T,xpath_t>, result_type>::type
    handle_attribute (T const& t) const {
      std::cout << this->indent() << "Has attributes" << std::endl;
    }

    template <typename T>
    typename boost::disable_if<has_children<T,xpath_t>,result_type>::type
    operator () (T const& t) const {
      // non-recursive
      std::cout << this->indent() << "Terminal Tag: " << tag(t, xpath_t()) << std::endl;
      return result_type();
    }

    template <typename T>
    bool apply_predicate (T const& t) const {
      // ignore the predicate for now
      return true;
    }

    template <typename T>
    typename boost::enable_if<has_children<T,xpath_t>,result_type>::type
    operator () (T const& t) const {
      // recursive
      typedef typename bel::iterator<T,xpath_t>::type iterator;
      if (this->path->size() <= this->axis)
        return result_type();
      result_type result_set;

      std::cout << this->indent() << "Recursive Tag: " << tag(t, xpath_t()) << std::endl;

      visitor<T> V(&t, this->path, axis+1, this);

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
          iterator first, last;
          boost::tie(first,last) = vpath::children(t, xpath_t());
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

    visitor_base const* parent;
    path_type const*    path;
    std::size_t         axis;
  };

  result_type operator () (path_t const& path, X const& gds) {
    visitor<X> V(&gds,&path);
    return visit(V, gds);
  }
};

template <typename X, typename String>
typename query_generator<String,X>::result_type
query (path::path_type<String> const& path, X const& x) {
  query_generator<String,X> qg;
  return qg(path, x);
}

template <typename X, typename String>
void query (String const& path, X const& gds) {
  query(path::path_type<String>(path), gds);
}

} // end vpath

#endif//VPATH_LIB_QUERY
