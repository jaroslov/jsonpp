// rdstl
#include <rdstl/rdstl.hpp>
// boost stuff
#include <boost/tuple/tuple.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/shared_ptr.hpp>
// STL stuff
#include <vector>
#include <set>
// local stuff
#include "concepts.hpp"
#include "path.hpp"

#ifndef XPGTL_LIB_QUERY
#define XPGTL_LIB_QUERY

namespace xpgtl {

  // a generic visitor for some unknown variant type;
  // the value of the variant is place into the reference_union
  // type (RU) by reference, when found
  template <typename RU>
  struct visit_node_ru {
    typedef RU result_type;
    template <typename T>
    RU operator () (T const& t) const {
      RU ru;
      ru = &t;
      return ru;
    }
    template <typename T>
    static RU go (T const& t) {
      visit_node_ru<RU> nru;
      return rdstl::visit(nru, t);
    }
  };

  //
  // We need a way to store iterators for any type we
  // find in the RDS. The RDSTL does not give us a
  // guarantee that there exists a reference-union that
  // can store any type of iterator we'll find. That
  // means we have to build an "abstract" iterator
  // that captures the interface to a forward iterator
  // (the kind of iterator we need).
  //
  // there are two implementations of the abstract
  // iterator: the node-iterator specialized for some type
  // "T" and for the type "void". A "T" specialized
  // iterator actually does work---it holds a real
  // iterator by value. The "void" specialized iterator
  // is a proxy for those types that do not have children.
  // The void-iterator always "fails".
  template <typename RU>
  struct abstract_iterator {
    abstract_iterator () : ru_store() {}
    abstract_iterator (RU const& ru) : ru_store(ru) {}
    virtual ~ abstract_iterator () {}
    virtual void next () = 0;
    virtual bool eq (abstract_iterator const&) const = 0;
    virtual bool n_eq (abstract_iterator const&) const = 0;
    virtual void dereference () = 0;
    virtual abstract_iterator<RU>* clone () const = 0;
    // these make the iterator "pretty"
    RU const& operator * () {
      this->dereference();
      return this->ru_store;
    }
    const RU* operator -> () {
      this->dereference();
      return &this->ru_store;
    }
    abstract_iterator& operator ++ () {
      this->next(); return *this;
    }
    friend bool operator == (abstract_iterator const& L, abstract_iterator const& R) {
      return L.eq(R);
    }
    friend bool operator != (abstract_iterator const& L, abstract_iterator const& R) {
      return L.n_eq(R);
    }

    RU ru_store;
  };
  template <typename RU, typename Iterator, typename Tag>
  struct node_iterator : public abstract_iterator<RU> {
    typedef abstract_iterator<RU>           base_type;
    typedef node_iterator<RU,Iterator,Tag>  self_type;
    node_iterator () : iterator() {}
    node_iterator (Iterator const& iter) : iterator(iter) {}
    node_iterator (Iterator const& iter, RU const& ru)
      : iterator(iter), abstract_iterator<RU>(ru) {}
    virtual ~ node_iterator () {}
    virtual void next () {
      ++this->iterator;
    }
    virtual bool eq (base_type const& atr) const {
      return this->iterator == static_cast<const self_type*>(&atr)->iterator;
    }
    virtual bool n_eq (base_type const& atr) const {
      return this->iterator != static_cast<const self_type*>(&atr)->iterator;
    }
    virtual void dereference () {
      this->ru_store = visit_node_ru<RU>::go(*this->iterator);
    }
    virtual abstract_iterator<RU>* clone () const {
      return new node_iterator<RU,Iterator,Tag>(this->iterator,this->ru_store);
    }

    Iterator iterator;
  };
  template <typename RU, typename Tag>
  struct node_iterator<RU,void,Tag> : public abstract_iterator<RU> {
    typedef abstract_iterator<RU>       base_type;
    virtual ~ node_iterator () {}
    virtual void next () {}
    virtual bool eq (base_type const&) const { return true; }
    virtual bool n_eq (base_type const&) const { return false; }
    virtual void dereference () {}
    virtual abstract_iterator<RU>* clone () const {
      return new node_iterator<RU,void,Tag>();
    }
  };

#ifdef XPGTL_DEBUG
  // this is an internal convenience visitor for debugging
  struct print_pointer {
    typedef void result_type;
    template <typename T>
    void operator () (T const&) const {
      std::cout << typeid(T).name() << std::endl;
    }
    void operator () (std::string const* str) const {
      std::cout << "\"" << *str << "\"" << std::flush;
    }
    void operator () (std::wstring const* wstr) const {
      std::wcout << "\"" << *wstr << "\"" << std::flush;
    }
    template <typename T>
    static void go (T const& t) {
      print_pointer pptr;
      rdstl::visit(pptr, t);
    }
  };
#endif//XPGTL_DEBUG

  //
  // Filters a variant so that it returns a pointer
  // or "0". That means that "TypeFilter" must be
  // of some dereferenceable type, and that "0"
  // is convertiable to that type.
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
  //   Q.next(as<foo>());
  // returns a result-set of "foo const*"
  template <typename ResultType>
  ResultType const* as () { return 0; }

  template <typename String, typename X, typename Tag=xpath<X> >
  struct Query {
    typedef typename rdstl::reference_union<X,Tag>::type    ru_type;
    typedef ru_type                                         value_type;
    typedef boost::shared_ptr<abstract_iterator<ru_type> >  sa_iter_t;

    struct node {
      struct build {
        typedef node result_type;
        template <typename T>
        node operator () (T const& t) const {
          return node();
        }
        template <typename T>
        static node go (ru_type const& ru) {
          build B;
          return rdstl::visit(B, ru);
        }
      };

      template <typename T>
      typename boost::disable_if<rdstl::has_children<T,Tag> >::type
      init_children (T const& t) {
        typedef node_iterator<ru_type,void,Tag> node_iter;
        this->children.first = sa_iter_t(new node_iter());
        this->children.second = sa_iter_t(new node_iter());
      }
      template <typename T>
      typename boost::enable_if<rdstl::has_children<T,Tag> >::type
      init_children (T const& t) {
        typedef typename bel::iterator<T,Tag>::type iter_t;
        typedef node_iterator<ru_type,iter_t,Tag> node_iter;
        iter_t f, l;
        boost::tie(f,l) = rdstl::children(t, Tag());
        this->children.first = sa_iter_t(new node_iter(f));
        this->children.first = sa_iter_t(new node_iter(l));
      }

      std::size_t path_index;
      axis_t::name_e alternate;
      ru_type reference;
      ru_type parent_reference;
      std::pair<sa_iter_t,sa_iter_t> children;
      std::pair<sa_iter_t,sa_iter_t> siblings;
      std::string tag_name;
    };

    Query (path<String> const& path, X const& x) {
      this->setup(path, x);
    }
    void setup (path<String> const& path, X const& x) {
      this->path = path;
      this->work.clear();
    }
    bool done () const {
      return this->work.empty();
    }
    template <typename FilterType>
    FilterType const* next (FilterType const*) {
      ru_type ru = this->next();
      filter_on_type<FilterType const*> fot;
      return rdstl::visit(fot, ru);
    }
    ru_type next () {
      while (not this->work.empty()) {
        node &it = this->work.back();
        if (this->path.size() <= it.path_index) {
#ifdef XPGTL_DEBUG
          std::cout << std::string(this->work.size()-1,'-') << ">"
            << this->work.back().tag_name << std::endl;
#endif//XPGTL_DEBUG
          ru_type ru = this->work.back().reference;
          this->work.pop_back();
          return ru;
        }
        axis_t const& axis = this->path[it.path_index];
        axis_t::name_e name
          = axis_t::unknown == it.alternate
              ? axis.name : it.alternate;
#ifdef XPGTL_DEBUG
        std::cout << std::string(this->work.size(),' ') << axis
          << " alt: " << (char)name << " idx: " << it.path_index
          << " " << this->path.test(it.path_index) << " ?= "
          << it.tag_name << std::endl;
#endif//XPGTL_DEBUG
        switch (name) {
        case axis_t::ancestor: this->handle_ancestor(it, axis); break;
        case axis_t::ancestor_or_self: this->handle_ancestor_or_self(it, axis); break;
        case axis_t::attribute: this->handle_attribute(it, axis); break;
        case axis_t::child: this->handle_child(it, axis); break;
        case axis_t::descendent: this->handle_descendent(it, axis); break;
        case axis_t::descendent_or_self: this->handle_descendent_or_self(it, axis); break;
        case axis_t::following: this->handle_following(it, axis); break;
        case axis_t::following_sibling: this->handle_following_sibling(it, axis); break;
        case axis_t::namespace_: this->handle_namespace(it, axis); break;
        case axis_t::parent: this->handle_parent(it, axis); break;
        case axis_t::preceding: this->handle_preceding(it, axis); break;
        case axis_t::preceding_sibling: this->handle_preceding_sibling(it, axis); break;
        case axis_t::self: this->handle_self(it, axis); break;
        default: throw std::runtime_error("Unrecognized axis-name.");
        }
      }
      return ru_type();
    }
    inline void handle_ancestor (node& it, axis_t const& axis) {
    }
    inline void handle_ancestor_or_self (node& it, axis_t const& axis) {
    }
    inline void handle_attribute (node& it, axis_t const& axis) {
    }
    inline void handle_child (node& it, axis_t const& axis) {
    }
    inline void handle_descendent (node& it, axis_t const& axis) {
    }
    inline void handle_descendent_or_self (node& it, axis_t const& axis) {
    }
    inline void handle_following (node& it, axis_t const& axis) {
    }
    inline void handle_following_sibling (node& it, axis_t const& axis) {
    }
    inline void handle_parent (node& it, axis_t const& axis) {
    }
    inline void handle_namespace (node& it, axis_t const& axis) {
    }
    inline void handle_preceding (node& it, axis_t const& axis) {
    }
    inline void handle_preceding_sibling (node& it, axis_t const& axis) {
    }
    inline void handle_self (node& it, axis_t const& axis) {
    }
    inline bool satisfies_test (node const& it, std::size_t idx) const {
    }

    std::vector<node> work;
    path<String> path;
  };

template <typename String, typename X>
std::set<typename Query<String,X>::value_type>
query (path<String> const& path, X const& x) {
  typedef typename Query<String,X>::value_type v_type;
  typedef std::set<v_type> r_set_t;
  r_set_t rset;
  Query<String,X> Q(path, x);
  while (not Q.done()) {
    v_type vt = Q.next();
    if (not Q.done())
      rset.insert(vt);
  }
  return rset;
}

template <typename String, typename X, typename FilterType>
std::set<FilterType const*>
query (path<String> const& path, X const& x, FilterType const*) {
  typedef std::set<FilterType const*> r_set_t;
  r_set_t rset;
  Query<String,X> Q(path, x);
  while (not Q.done()) {
    FilterType const* ft = Q.next(as<FilterType>());
    if (not Q.done())
      rset.insert(ft);
  }
  return rset;
}

} // end xpgtl

#endif//XPGTL_LIB_QUERY
