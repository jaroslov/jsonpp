// rdstl
#include <rdstl/rdstl.hpp>
// boost stuff
#include <boost/tuple/tuple.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/shared_ptr.hpp>
// STL stuff
#include <vector>
// local stuff
#include "concepts.hpp"
#include "path.hpp"

#ifndef XPGTL_LIB_QUERY
#define XPGTL_LIB_QUERY

namespace xpgtl {

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

  struct print_pointer {
    typedef void result_type;
    template <typename T>
    void operator () (T const&) const {}
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

  template <typename String, typename X, typename Tag=xpath<X> >
  struct Query {
    typedef typename rdstl::reference_union<X,Tag>::type    ru_type;
    typedef boost::shared_ptr<abstract_iterator<ru_type> >  sa_iter_t;

    struct build_item_ptr;
    struct item {
      item () : node() {}
      item (item const& it) {
        this->copy(it);
      }
      item& operator = (item const& it) {
        this->copy(it);
        return *this;
      }
      void copy (item const& it) {
        this->alternate = it.alternate;
        this->index     = it.index;
        this->begin     = it.begin;
        this->current   = it.current;
        this->end       = it.end;
        this->tag_name  = it.tag_name;
        this->node      = it.node;
        this->forebear  = it.forebear;
        this->knows_forebear = it.knows_forebear;
      }
      template <typename T>
      item (T const& t, axis_t::name_e name=axis_t::unknown,
        std::size_t idx=0, ru_type const& parent=ru_type()) {
        this->alternate = name;
        this->index = idx;
        this->init_children(t);
        this->init_parent(t, parent);
      }
      template <typename T>
      typename boost::disable_if<rdstl::has_children<T,Tag> >::type
      init_children (T const& t) {
        typedef node_iterator<ru_type,void,Tag> node_iter;
        this->tag_name = tag(t, Tag());
        this->node = &t;
        this->begin = sa_iter_t(new node_iter());
        this->current = sa_iter_t(new node_iter());
        this->end = sa_iter_t(new node_iter());
      }
      template <typename T>
      typename boost::enable_if<rdstl::has_children<T,Tag> >::type
      init_children (T const& t) {
        typedef typename bel::iterator<T,Tag>::type iter_t;
        typedef node_iterator<ru_type,iter_t,Tag> node_iter;
        this->tag_name = tag(t, Tag());
        this->node = &t;
        iter_t f, l;
        boost::tie(f,l) = rdstl::children(t, Tag());
        this->begin = sa_iter_t(new node_iter(f));
        this->current = sa_iter_t(new node_iter(f));
        this->end = sa_iter_t(new node_iter(l));
      }
      template <typename T>
      typename boost::disable_if<rdstl::knows_parent<T,Tag> >::type
      init_parent (T const& t, ru_type const& parent) {
        this->forebear = parent;
        this->knows_forebear = false;
      }
      template <typename T>
      typename boost::enable_if<rdstl::knows_parent<T,Tag> >::type
      init_parent (T const& t, ru_type const&) {
        this->forebear = parent(t, Tag());
        this->knows_forebear = true;
      }

      axis_t::name_e alternate;
      std::size_t index;
      std::string tag_name;
      sa_iter_t begin, current, end;
      ru_type node, forebear;
      bool knows_forebear;
    };
    struct build_item {
      typedef item result_type;
      template <typename T>
      item operator () (T const& t) const {
        return item(t, this->name, this->index, this->parent);
      }
      template <typename T>
      item operator () (T const* t) const {
        return item(*t, this->name, this->index, this->parent);
      }
      template <typename T>
      item operator () (rdstl::valued<T> const& t) const {
        return item(*t, this->name, this->index, this->parent);
      }
      template <typename T>
      static item go (T const& t, axis_t::name_e name=axis_t::unknown,
        std::size_t idx=0, ru_type const& parent=ru_type()) {
        build_item bi;
        bi.parent = parent;
        bi.name = name;
        bi.index = idx;
        return rdstl::visit(bi, t);
      }
      ru_type parent;
      axis_t::name_e name;
      std::size_t index;
    };

    Query (path<String> const& path, X const& x) {
      this->setup(path, x);
    }
    void setup (path<String> const& path, X const& x) {
      this->path = path;
      this->work.clear();
      this->work.push_back(build_item::go(x));
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
        item &it = this->work.back();
        if (this->path.size() <= it.index) {
          ru_type ru = this->work.back().node;
          this->work.pop_back();
          return ru;
        }
        axis_t const& axis = this->path[it.index];
        axis_t::name_e name
          = axis_t::unknown == it.alternate
              ? axis.name : it.alternate;
        switch (name) {
        case axis_t::ancestor: this->handle_ancestor(it, axis); break;
        case axis_t::ancestor_or_self: this->handle_ancestor_or_self(it, axis); break;
        case axis_t::attribute: break;
        case axis_t::child: this->handle_child(it, axis); break;
        case axis_t::descendent: this->handle_descendent(it, axis); break;
        case axis_t::descendent_or_self: this->handle_descendent_or_self(it, axis); break;
        case axis_t::following: break;
        case axis_t::following_sibling: break;
        case axis_t::namespace_: break;
        case axis_t::parent: this->handle_parent(it, axis); break;
        case axis_t::preceding: break;
        case axis_t::preceding_sibling: break;
        case axis_t::self: this->handle_self(it, axis); break;
        default: throw std::runtime_error("Unrecognized axis-name.");
        }
      }
      return ru_type();
    }
    inline void handle_ancestor (item& it, axis_t const& axis) {
      if (it.knows_forebear or this->work.size() > 1) {
        item ntm = build_item::go(it.forebear, axis_t::parent, it.index);
        item mtm = build_item::go(it.forebear, axis_t::ancestor, it.index);
        this->work.push_back(ntm);
        this->work.push_back(mtm);
      }
    }
    inline void handle_ancestor_or_self (item& it, axis_t const& axis) {
      it.alternate = axis_t::self;
      this->handle_ancestor(it, axis);
    }
    inline void handle_child (item& it, axis_t const& axis) {
      // add a child and call it a self as a proxy
      if (*it.current != *it.end) {
        item ntm = build_item::go(**it.current, axis_t::self, it.index);
        ++*it.current;
        this->work.push_back(ntm);
      } else
        this->work.pop_back();
    }
    inline void handle_descendent (item& it, axis_t const& axis) {
      if (*it.current != *it.end) {
        // add two items: "child" and "descendent"
        item ntm = build_item::go(**it.current, axis_t::self, it.index);
        item mtm = build_item::go(**it.current, axis_t::descendent, it.index);
        ++*it.current;
        // don't invalidate "item"
        this->work.push_back(ntm);
        this->work.push_back(mtm);
      } else
        this->work.pop_back();
    }
    inline void handle_descendent_or_self (item& it, axis_t const& axis) {
      // change self to point at "me"
      // and look at all descendents
      it.alternate = axis_t::self;
      this->work.push_back(build_item::go(it.node, axis_t::descendent, it.index));
    }
    inline void handle_parent (item& it, axis_t const& axis) {
      // either we have built-in parent support, or we are at least
      // the second item in the work-list (if we're the first item
      // and we don't have parent-support, we could have a false-positive
      // if the first type in the variant is a "valued<*>" type)
      if (it.knows_forebear or this->work.size() > 1) {
        this->work.push_back(build_item::go(it.forebear, axis_t::self, it.index));
      }
    }
    inline void handle_self (item& it, axis_t const& axis) {
      if (it.tag_name == this->path.test(axis)) {
        ++it.index;
        it.alternate = axis_t::unknown;
      } else
        this->work.pop_back();
    }

    std::vector<item> work;
    path<String> path;
  };

} // end xpgtl

#endif//XPGTL_LIB_QUERY
