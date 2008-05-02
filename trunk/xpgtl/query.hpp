// local stuff
#include "concepts.hpp"
#include "path.hpp"
// rdstl
#include <rdstl/rdstl.hpp>
// boost stuff
#include <boost/tuple/tuple.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>
// STL stuff
#include <set>
#include <stdexcept>

#ifndef XPGTL_LIB_QUERY
#define XPGTL_LIB_QUERY

namespace xpgtl {

template <typename String, typename X>
struct query_generator {
  typedef xpgtl::path<String>               path_t;
  typedef xpath<X>                          xpath_t;
  typedef rdstl::reference_union<X,xpath_t> ru_mf;
  typedef typename ru_mf::type              r_union_t;
  typedef std::set<r_union_t>               result_set_t;
  typedef query_generator<String,X>         qg_type;

  struct visitor_base {
    virtual void visit_parent () const = 0;
  };

  // We don't actually care about "Node" but explicit instantiations
  // are illegal, whereas partial specialization is fine; another
  // WTFism from C++
  template <typename Node, typename Iterator>
  struct sibling_base {

    template <typename Visitor>
    void handle_preceding_sibling (Visitor const& V) const {
      // set the current axis name to "self"
      const axis_t::name_e old_name = (*V.path)[this->axis].name;
      V.path->axes[this->axis].name = axis_t::self;
      for (Iterator iter=this->first; iter!=this->self; ++iter)
        rdstl::visit(V, *iter);
      // unset the axis name
      V.path->axes[this->axis].name = old_name;
    }

    template <typename Visitor>
    void handle_following_sibling (Visitor const& V) const {
      if (this->self == this->last)
        return;
      // set the current axis name to "self"
      const axis_t::name_e old_name = (*V.path)[this->axis].name;
      V.path->axes[this->axis].name = axis_t::self;
      Iterator iter = self; ++iter;
      for ( ; iter!=this->last; ++iter)
        rdstl::visit(V, *iter);
      // unset the axis name
      V.path->axes[this->axis].name = old_name;
    }

    void set_siblings (Iterator first, Iterator self, Iterator last) {
      this->first = first;
      this->self  = self;
      this->last  = last;
    }
    Iterator first, self, last;
  };
  template <typename Node> struct sibling_base<Node,void> {
    template <typename Visitor>
    void handle_preceding_sibling (Visitor const& V) const {}
    template <typename Visitor>
    void handle_following_sibling (Visitor const& V) const {}
    template <typename Iter>
    void set_siblings (Iter, Iter, Iter);
  };

  template <typename Node, typename Iterator=void>
  struct visitor : public visitor_base, sibling_base<Node,Iterator> {
    typedef Iterator      sibling_iterator;
    typedef visitor<Node> self_type;
    typedef Node          node_type;
    typedef void          result_type;

    virtual void visit_parent () const {
      (*this)(*this->node);
    }

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
      : path(path), axis(axis), node(node)
      , parent(parent), result_set(rset) {
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
    typename boost::enable_if<rdstl::has_children<T,xpath_t> >::type
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
      for (boost::tie(first,last)=rdstl::children(t, xpath_t()); first!=last; ++first)
        rdstl::visit(V, *first);
      // unset the axis name
      this->path->axes[this->axis].name = old_name;
    }
    template <typename T>
    typename boost::disable_if<rdstl::has_children<T,xpath_t> >::type
    handle_children (T const& t) const {
      // if an element has no children, we return nothing
    }
    template <typename T>
    typename boost::enable_if<rdstl::knows_parent<T,xpath_t> >::type
    handle_parent_internal_ (T const& t) const {
      // real parent support; the parent must be a pointer,
      // and it will be visitable with children
      (*this)(*rdstl::parent(t, xpath_t()));
    }
    template <typename T>
    typename boost::disable_if<rdstl::knows_parent<T,xpath_t> >::type
    handle_parent_internal_ (T const& t) const {
      // real parent support; the parent must be a pointer,
      // and it will be visitable with children
      this->parent->visit_parent();
    }
    template <typename T>
    void handle_parent (T const& t) const {
      // emulated parent support
      if (0 == this->parent)
        return;
      // set the current axis name to "self"
      const axis_t::name_e old_name = (*this->path)[this->axis].name;
      this->path->axes[this->axis].name = axis_t::self;
      this->handle_parent_internal_(t);
      // unset the axis name
      this->path->axes[this->axis].name = old_name;
    }

    template <typename T>
    void handle_self (T const& t) const {
      const xpgtl::axis_t Axis = (*this->path)[this->axis];
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
    typename boost::enable_if<rdstl::has_children<T,xpath_t> >::type
    handle_descendent (T const& t) const {
      this->handle_children(t); // handle the children
      // now, recurse on the children and tell them to
      // do the same!

      // we only look at descendents, never self
      const axis_t::name_e old_name = (*this->path)[this->axis].name;
      this->path->axes[this->axis].name = axis_t::descendent;

      typedef typename bel::iterator<T,xpath<X> >::type ch_iter;
      ch_iter first, last;
      visitor<T> V(&t, this->path, this->axis, this, this->result_set);
#ifdef XPGTL_DEBUG
      V.recursion_depth = this->recursion_depth+1;
#endif//XPGTL_DEBUG
      for (boost::tie(first,last)=rdstl::children(t, xpath_t()); first!=last; ++first)
        rdstl::visit(V, *first);
      // put original name on the path-stack
      this->path->axes[this->axis].name = old_name;
    }
    template <typename T>
    typename boost::disable_if<rdstl::has_children<T,xpath_t> >::type
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
    void handle_ancestor (T const& t) const {
      // we call the "handle-parent" function which essentially goes
      // to the parent while translating into a "self" call
      this->handle_parent(t);
      // Now, we call the parent directly, while setting the current
      // stack to "ancestor"; this will recurse up the tree calling
      // "handle-parent", resolving to "self" each time, and calling
      // all of the parents
      const axis_t::name_e old_name = (*this->path)[this->axis].name;
      this->path->axes[this->axis].name = axis_t::ancestor;
      this->handle_parent_internal_(t);
      // put original name on the path-stack
      this->path->axes[this->axis].name = old_name;
    }
    template <typename T>
    void handle_ancestor_or_self (T const& t) const {
      // union of self & ancestor handlers
      // we don't have to modify the stack, because handle-self
      // operates without any recursion (as the context-node)
      // and ancestor replaces axis-name with the "ancestor" axis
      this->handle_self(t);
      this->handle_ancestor(t);
    }

    template <typename T>
    void handle_preceding (T const& t) const {
      throw std::runtime_error("Must move a stack-based system.");
    }

    template <typename T>
    void handle_following (T const& t) const {
      throw std::runtime_error("Must move a stack-based system.");
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
    typename boost::disable_if<rdstl::is_proxy<T,xpath_t> >::type
    add_to_result_set (T const& t) const {
      this->result_set->insert(&t);
#ifdef XPGTL_DEBUG
      this->print(t);
#endif//XPGTL_DEBUG
    }
    template <typename T>
    typename boost::enable_if<rdstl::is_proxy<T,xpath_t> >::type
    add_to_result_set (T const& t) const {
#ifdef XPGTL_DEBUG
      std::cout << "NORESULT" << std::endl;
#endif//XPGTL_DEBUG
    }    

    template <typename T>
    void operator () (T const& t) const {

      if (this->path->size() <= this->axis) {
        // we trivially return the input if we have no test
        this->add_to_result_set(t);
        return;
      }

      const xpgtl::axis_t Axis = (*this->path)[this->axis];

#ifdef XPGTL_DEBUG
      std::cout << this->indent() << Axis << std::endl;
#endif//XPGTL_DEBUG
  
      switch (Axis.name) {
      case axis_t::ancestor: this->handle_ancestor(t); break;
      case axis_t::ancestor_or_self: this->handle_ancestor_or_self(t); break;
      case axis_t::attribute: this->handle_attribute(t); break;
      case axis_t::child: this->handle_children(t); break;
      case axis_t::descendent: this->handle_descendent(t); break;
      case axis_t::descendent_or_self: this->handle_descendent_or_self(t); break;
      case axis_t::following: this->handle_following(t); break;
      case axis_t::following_sibling: this->handle_following_sibling(*this); break;
      case axis_t::namespace_: {
          // honestly, this shit makes no sense
          throw std::runtime_error("Unsupported axis-name: namespace");
        } break;
      case axis_t::parent: this->handle_parent(t); break;
      case axis_t::preceding: this->handle_preceding(t); break;
      case axis_t::preceding_sibling: this->handle_preceding_sibling(*this); break;
      case axis_t::self: this->handle_self(t); break;
      default: std::runtime_error("Unrecognized axis-name.");
      }
    }

#ifdef XPGTL_DEBUG
    std::size_t recursion_depth;
#endif//XPGTL_DEBUG
    // Members
    node_type const*    node;
    visitor_base const* parent;
    path_t*             path;
    std::size_t         axis;
    result_set_t*       result_set;
  };

  result_set_t operator () (path_t& path, X const& gds) {
    result_set_t result_set;
    visitor<X> V(&gds,&path,0,0,&result_set);
    rdstl::visit(V, gds);
    return result_set;
  }
};

template <typename X, typename String>
typename query_generator<String,X>::result_set_t
query (xpgtl::path<String> const& path, X const& x) {
  xpgtl::path<String> lpath(path);
  query_generator<String,X> qg;
  return qg(lpath, x);
}

template <typename X, typename String>
typename query_generator<String,X>::result_set_t
query (String const& path, X const& gds) {
  return query(xpgtl::path<String>(path), gds);
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

//
// this is a convenience function to give us BASIC-like syntax:
//   query(path, datastructure, as<foo>());
// returns a result-set of "foo const*"
template <typename ResultType>
ResultType const* as () { return 0; }

template <typename ResultType, typename X, typename String>
std::set<const ResultType*>
query (String const& path, X const& x, ResultType const* rt=0) {
  typedef std::set<const ResultType*> rset_t;
  typedef typename query_generator<String,X>::result_set_t qset_t;
  typedef typename qset_t::const_iterator q_iter;

  qset_t qset = query(path, x);

  rset_t rset;
  q_iter first, last;
  filter_on_type<const ResultType*> fot;
  for (boost::tie(first,last) = bel::sequence(qset); first!=last; ++first) {
    ResultType const* possible = rdstl::visit(fot,*first);
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
  rdstl::visit(vrt, rtype);
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

} // end xpgtl

#endif//XPGTL_LIB_QUERY
