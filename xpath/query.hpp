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
  typedef vpath::path<String>             path_t;
  typedef xpath<X>                        xpath_t;
  typedef vpath::has_children<X,xpath_t>  cu_mf;
  typedef typename cu_mf::type            c_union_t;
  typedef std::vector<c_union_t>          result_type;
  typedef query_generator<String,X>       qg_type;

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
    typedef Node                          node_type;
    typedef typename qg_type::result_type result_type;

    std::string indent () const {
      return std::string(2*this->axis,' ');
    }

    visitor (node_type const* node=0, path_t const* path=0,
      std::size_t axis=0, visitor_base const* parent=0)
      : path(path), axis(axis), parent(parent), use_alternate(false) {
      this->node() = node;
    }

    template <typename T>
    typename boost::disable_if<has_attributes<T,xpath_t>,result_type>::type
    handle_attribute (T const& t) const {
      throw std::runtime_error("(no attributes) Attributes are not implemented");
    }
    template <typename T>
    typename boost::enable_if<has_attributes<T,xpath_t>,result_type>::type
    handle_attribute (T const& t) const {
      throw std::runtime_error("(has attributes) Attributes are not implemented");
    }

    template <typename T>
    typename boost::enable_if<has_children<T,xpath_t>,result_type>::type
    handle_children (T const& t) const {
      const vpath::axis_t Axis = (*this->path)[this->axis];
      const String axis_test   = this->path->test(this->axis);
      const String self_tag    = tag(t, xpath_t());

      result_type result_set;

      typedef typename bel::iterator<T,xpath<X> >::type ch_iter;
      ch_iter first, last;
      visitor<T> V(&t, this->path, this->axis, this);
      V.use_alternate = true;
      V.alternate = vpath::axis_t((*this->path)[this->axis]);
      V.alternate.name = vpath::axis_t::self;
      for (boost::tie(first,last)=bel::sequence(t, xpath_t()); first!=last; ++first) {
        result_type lset = visit(V, *first);
        result_set.insert(result_set.end(), lset.begin(), lset.end());
      }

      return result_set;
    }

    template <typename T>
    typename boost::disable_if<has_children<T,xpath_t>,result_type>::type
    handle_children (T const& t) const {
      // if an element has no children, we return nothing
      return result_type();
    }

    template <typename T>
    result_type handle_self (T const& t) const {
      const vpath::axis_t Axis = (*this->path)[this->axis];
      const String axis_test   = this->path->test(this->axis);
      const String self_tag    = tag(t, xpath_t());

      if (self_tag == axis_test
        or (Axis.function and "node" == axis_test)) {
        // if we match, then we try to get the result set remaindered
        visitor<T> V(&t, this->path, this->axis+1, this);
        return V(t);
      }
      return result_type();
    }

    template <typename T>
    bool apply_predicate (T const& t) const {
      // ignore the predicate for now
      return true;
    }

    template <typename T>
    result_type operator () (T const& t) const {
      result_type result_set;

      if (this->path->size() <= this->axis) {
        // we trivially return the input if we have no test
        result_set.push_back(&t);
        return result_set;
      }

      const vpath::axis_t Axis = this->use_alternate?this->alternate:(*this->path)[this->axis];
      const String axis_test   = this->path->test(Axis);
      const String self_tag    = tag(t, xpath_t());

      visitor<T> V(&t, this->path, axis+1, this);

      switch (Axis.name) {
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
          result_set = this->handle_children(t);
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
          result_set = this->handle_self(t);
        } break;
      default: {
          std::cout << "Unsupported axis-name." << std::endl;
        }
      }
      return result_set;
    }

    visitor_base const* parent;
    path_t const*       path;
    std::size_t         axis;
    bool                use_alternate;
    vpath::axis_t       alternate;
  };

  result_type operator () (path_t const& path, X const& gds) {
    visitor<X> V(&gds,&path);
    return visit(V, gds);
  }
};

template <typename X, typename String>
typename query_generator<String,X>::result_type
query (vpath::path<String> const& path, X const& x) {
  query_generator<String,X> qg;
  return qg(path, x);
}

template <typename X, typename String>
typename query_generator<String,X>::result_type
query (String const& path, X const& gds) {
  return query(vpath::path<String>(path), gds);
}

struct visit_result_type {
  typedef void result_type;
  template <typename T>
  void operator () (T const& t) const {
    std::cout << std::hex << t;
  }
};

template <typename ResultType>
void print_result_set (ResultType const& rtype) {
  visit_result_type vrt;
  visit(vrt, rtype);
}

template <typename ResultType>
void print_result_set (std::vector<ResultType> const& result_set) {
  std::cout << "[";
  for (std::size_t i=0; i<result_set.size(); ++i) {
    print_result_set(result_set[i]);
    if ((i+1) < result_set.size())
      std::cout << ", ";
  }
  std::cout << "]";
}

} // end vpath

#endif//VPATH_LIB_QUERY
