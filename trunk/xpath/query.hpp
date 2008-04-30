// local stuff
#include "concepts.hpp"
#include "path.hpp"
// boost stuff
#include <boost/tuple/tuple.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>
// STL stuff
#include <set>
#include <stdexcept>

#ifndef XPGTL_LIB_QUERY
#define XPGTL_LIB_QUERY

namespace vpath {

template <typename String, typename X>
struct query_generator {
  typedef vpath::path<String>             path_t;
  typedef xpath<X>                        xpath_t;
  typedef vpath::has_children<X,xpath_t>  cu_mf;
  typedef typename cu_mf::type            c_union_t;
  typedef std::set<c_union_t>             result_set_t;
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
    typedef visitor<Node> self_type;
    typedef Node          node_type;
    typedef void          result_type;

#ifdef XPGTL_DEBUG
    std::string indent (char rep=' ', char head=' ') const {
      std::size_t depth = 2*this->recursion_depth;
      if (depth > 0)
        depth -= 1;
      return std::string(depth,rep) + (depth>0?std::string(1,head):"");
    }
#endif//XPGTL_DEBUG

    visitor (node_type const* node=0, path_t* path=0,
      std::size_t axis=0, visitor_base const* parent=0,
      result_set_t* rset=0)
      : path(path), axis(axis)
      , parent(parent), result_set(rset) {
      this->node() = node;
#ifdef XPGTL_DEBUG
      this->recursion_depth = 0;
#endif//XPGTL_DEBUG
    }

    template <typename T>
    typename boost::disable_if<has_attributes<T,xpath_t> >::type
    handle_attribute (T const& t) const {
      // do nothing
    }
    template <typename T>
    typename boost::enable_if<has_attributes<T,xpath_t> >::type
    handle_attribute (T const& t) const {
      throw std::runtime_error("(has attributes) Attributes are not implemented");
    }

    template <typename T>
    typename boost::enable_if<has_children<T,xpath_t> >::type
    handle_children (T const& t, bool mask=true) const {
      // mask: whether to mask the child as self
      //  we don't do this when handle_children is being used
      //  for "descendent" or "descendent-or-self"
      typedef typename bel::iterator<T,xpath<X> >::type ch_iter;
      ch_iter first, last;
      visitor<T> V(&t, this->path, this->axis, this, this->result_set);
#ifdef XPGTL_DEBUG
      V.recursion_depth = this->recursion_depth+1;
#endif//XPGTL_DEBUG

      // set the current axis name to "self"
      const axis_t::name_e old_name = (*this->path)[this->axis].name;
      this->path->axes[this->axis].name = axis_t::self;
      for (boost::tie(first,last)=bel::sequence(t, xpath_t()); first!=last; ++first)
        visit(V, *first);
      // unset the axis name
      this->path->axes[this->axis].name = old_name;
    }
    template <typename T>
    typename boost::disable_if<has_children<T,xpath_t> >::type
    handle_children (T const& t) const {
      // if an element has no children, we return nothing
    }

    template <typename T>
    void handle_self (T const& t) const {
      const vpath::axis_t Axis = (*this->path)[this->axis];
      const String axis_test   = this->path->test(this->axis);
      const String self_tag    = tag(t, xpath_t());

      if (self_tag == axis_test
        or (Axis.function and "node" == axis_test)) {
        visitor<T> V(&t, this->path, this->axis+1, this, this->result_set);
#ifdef XPGTL_DEBUG
        V.recursion_depth = this->recursion_depth+1;
#endif//XPGTL_DEBUG
        V(t);
      }
    }

    template <typename T>
    typename boost::enable_if<has_children<T,xpath_t> >::type
    handle_descendent (T const& t) const {
      this->handle_children(t); // handle the children
      // now, recurse on the children and tell them to
      // do the same!
      const vpath::axis_t Axis = (*this->path)[this->axis];
      const String axis_test   = this->path->test(this->axis);
      const String self_tag    = tag(t, xpath_t());

      // we only look at descendents, never self
      const axis_t::name_e old_name = (*this->path)[this->axis].name;
      this->path->axes[this->axis].name = axis_t::descendent;

      typedef typename bel::iterator<T,xpath<X> >::type ch_iter;
      ch_iter first, last;
      visitor<T> V(&t, this->path, this->axis, this, this->result_set);
#ifdef XPGTL_DEBUG
      V.recursion_depth = this->recursion_depth+1;
#endif//XPGTL_DEBUG
      for (boost::tie(first,last)=bel::sequence(t, xpath_t()); first!=last; ++first)
        visit(V, *first);
      // put original name on the path-stack
      this->path->axes[this->axis].name = old_name;
    }
    template <typename T>
    typename boost::disable_if<has_children<T,xpath_t> >::type
    handle_descendent (T const& t) const {
      // if an element has no children, we return nothing
    }

    template <typename T>
    void handle_descendent_or_self (T const& t) const {
      // a union of self & descendent handlers
      this->handle_self(t);
      this->handle_descendent(t);
    }

    template <typename T>
    bool apply_predicate (T const& t) const {
      // ignore the predicate for now
      return true;
    }

#ifdef XPGTL_DEBUG
    template <typename T>
    void print (T const& t) const {
      std::cout << this->indent('-','>') << "Not a string" << std::endl;
    }
    template <typename Char>
    void print (std::basic_string<Char> const& str) const {
      std::string Str(str.begin(), str.end());
      std::cout << this->indent('-','>') << "String: " << Str << std::endl;
    }
#endif//XPGTL_DEBUG

    template <typename T>
    void operator () (T const& t) const {

      if (this->path->size() <= this->axis) {
        // we trivially return the input if we have no test
        this->result_set->insert(&t);
#ifdef XPGTL_DEBUG
        this->print(t);
#endif//XPGTL_DEBUG
        return;
      }

      const vpath::axis_t Axis = (*this->path)[this->axis];

#ifdef XPGTL_DEBUG
      std::cout << this->indent() << Axis << std::endl;
#endif//XPGTL_DEBUG
  
      switch (Axis.name) {
      case axis_t::ancestor: {
          throw std::runtime_error("Unsupported axis-name: ancestor");
        } break;
      case axis_t::ancestor_or_self: {
          throw std::runtime_error("Unsupported axis-name: ancestor-or-self");
        } break;
      case axis_t::attribute: this->handle_attribute(t); break;
      case axis_t::child: this->handle_children(t); break;
      case axis_t::descendent: this->handle_descendent(t); break;
      case axis_t::descendent_or_self: this->handle_descendent_or_self(t); break;
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
      case axis_t::self: this->handle_self(t); break;
      default: std::runtime_error("Unrecognized axis-name.");
      }
    }

#ifdef XPGTL_DEBUG
    std::size_t recursion_depth;
#endif//XPGTL_DEBUG
    // Members
    visitor_base const* parent;
    path_t*             path;
    std::size_t         axis;
    result_set_t*       result_set;
  };

  result_set_t operator () (path_t& path, X const& gds) {
    result_set_t result_set;
    visitor<X> V(&gds,&path,0,0,&result_set);
    visit(V, gds);
    return result_set;
  }
};

template <typename X, typename String>
typename query_generator<String,X>::result_set_t
query (vpath::path<String> const& path, X const& x) {
  vpath::path<String> lpath(path);
  query_generator<String,X> qg;
  return qg(lpath, x);
}

template <typename X, typename String>
typename query_generator<String,X>::result_set_t
query (String const& path, X const& gds) {
  return query(vpath::path<String>(path), gds);
}

template <typename TypeFilter>
struct filter_on_type {
  typedef TypeFilter result_type;

  result_type operator () (TypeFilter const& t) const {
    return t;
  }
  template <typename T>
  result_type operator () (T const& t) const {
    return 0;
  }
};

template <typename ResultType, typename X, typename String>
std::set<const ResultType*> query (String const& path, X const& x, ResultType const* rt=0) {
  typedef std::set<const ResultType*> rset_t;
  typedef typename query_generator<String,X>::result_set_t qset_t;
  typedef typename qset_t::const_iterator q_iter;

  qset_t qset = query(path, x);

  rset_t rset;
  q_iter first, last;
  filter_on_type<const ResultType*> fot;
  for (boost::tie(first,last) = bel::sequence(qset); first!=last; ++first) {
    ResultType const* possible = visit(fot,*first);
    if (0 != possible)
      rset.insert(possible);
  }

  return rset;
}

struct visit_result_set {
  typedef void result_type;
  template <typename T>
  void operator () (T const& t) const {
    std::cout << std::hex << t;
  }
};

template <typename ResultType>
void print_result_set (ResultType const& rtype) {
  visit_result_set vrt;
  visit(vrt, rtype);
}

template <typename ResultType>
void print_result_set (std::set<ResultType> const& result_set) {
  typedef typename std::set<ResultType>::const_iterator iter;
  iter first, last;
  boost::tie(first,last)=bel::sequence(result_set);
  std::cout << "[";
  if (first != last) {
    print_result_set(*first);
    ++first;
  }
  for ( ;first!=last; ++first) {
    std::cout << ", ";
    print_result_set(*first);
  }
  std::cout << "]";
}

} // end vpath

#endif//XPGTL_LIB_QUERY
