#include "vpath.hpp"
#include <begin-end.hpp>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <boost/variant.hpp>
#include <boost/utility/enable_if.hpp>

#ifndef VPATH_LIB_BASICS
#define VPATH_LIB_BASICS

namespace vpath {

// standard (built-in) tags
template <typename X> std::string tag (bool, xpath<X>) { return "bool"; }
template <typename X> std::string tag (signed char, xpath<X>) { return "schar"; }
template <typename X> std::string tag (unsigned char, xpath<X>) { return "uchar"; }
template <typename X> std::string tag (wchar_t, xpath<X>) { return "wchar_t"; }
template <typename X> std::string tag (signed short, xpath<X>) { return "sshort"; }
template <typename X> std::string tag (unsigned short, xpath<X>) { return "ushort"; }
template <typename X> std::string tag (signed int, xpath<X>) { return "sint"; }
template <typename X> std::string tag (unsigned int, xpath<X>) { return "uint"; }
template <typename X> std::string tag (signed long, xpath<X>) { return "slong"; }
template <typename X> std::string tag (unsigned long, xpath<X>) { return "ulong"; }
template <typename X> std::string tag (float, xpath<X>) { return "float"; }
template <typename X> std::string tag (double, xpath<X>) { return "double"; }

template <typename T, typename A, typename X>
std::string tag (std::list<T,A> const& v, xpath<X> x) {
  return "list";
}
template <typename K, typename V, typename C, typename A, typename X>
std::string tag (std::map<K,V,C,A> const& v, xpath<X> x) {
  return "map";
}
template <typename V, typename C, typename A, typename X>
std::string tag (std::set<V,C,A> const& v, xpath<X> x) {
  return "set";
}
template <typename T, typename X>
std::string tag (std::basic_string<T> const& str, xpath<X> x) {
  return "string";
}
template <typename T, typename A, typename X>
std::string tag (std::vector<T,A> const& v, xpath<X> x) {
  return "vector";
}

// standard (or builtin) recursive types
template <typename T, typename A>
struct recursive<std::list<T,A> > : boost::mpl::true_ {};
//template <typename K, typename V, typename C, typename A>
//struct recursive<std::map<K,V,C,A> > : boost::mpl::true_ {};
template <typename K, typename C, typename A>
struct recursive<std::set<K,C,A> > : boost::mpl::true_ {};
template <typename T, typename A>
struct recursive<std::vector<T,A> > : boost::mpl::true_ {};

// key-value iterator facade
// 1. the key-value facade must return a boost::variant of itself for simplicity
// 2. the key-value facade must maintain its key (not just value!)
// 3. the key-value facade must dereference to something for which the
//    "sequence" call, calls on the value

// IDEA:
//  1. sequence(map) -> <iter,iter>
//  2. *iter -> variant<facade>
//  3. sequence(facade) -> <iter,iter>
//  4. *iter -> map::mapped_type
// WHY?
//  1. this allows us to interrupt and grab the "key" as a possible
//      attribute
template <typename T> // T had best be a variant!
struct trivial_iterator {
  typedef trivial_iterator<T> self_type;

  trivial_iterator (bool atend=false, T const* value=0)
    : end(atend), value_(value) {}
  trivial_iterator (self_type const& st) : end(st.end), value_(st.value_) {}
  self_type& operator = (self_type const& st) {
    this->value_ = st.value_; return *this;
  }
  self_type& operator ++ () {
    this->end = false;
    return *this;
  }
  self_type operator ++ () {
    self_type cp(*this);
    ++(*this);
    return cp;
  }
  T const& operator * () const { return *this->value_; }
  T const* operator -> () const { return this->value_; }

  friend bool operator == (self_type const& L, self_type const& R) {
    return (L.end == R.end) and (L.value_ == R.value_);
  }
  friend bool operator != (self_type const& L, self_type const& R) {
    return (L.end != R.end) or (L.value_ != R.value_);
  }

  bool end;
  T const* value_;
};

template <typename AssociativeContainer>
struct assoc_ctr_value_facade {
  typedef AssociativeContainer                          associative_container;
  typedef assoc_ctr_value_facade<AssociativeContainer>  self_type;

  typedef typename AssociativeContainer::key_type       ac_key_type;
  typedef typename AssociativeContainer::mapped_type    ac_mapped_type;
  typedef typename AssociativeContainer::value_type     ac_value_type;

  typedef trivial_iterator<ac_mapped_type>              iterator;
  typedef trivial_iterator<ac_mapped_type>              const_iterator;

  assoc_ctr_value_facade () {}
  assoc_ctr_value_facade (ac_value_type const& v) : value_(v) {}
  assoc_ctr_value_facade (self_type const& acvf) : value_(acvf.value_) {}
  self_type& operator = (self_type const& acvf) {
    this->value_ = acvf.value_;
    return *this;
  }
  self_type& operator = (ac_value_type const& acvt) {
    this->value_ = acvt;
    return *this;
  }

  iterator begin () const {
    return iterator(false,&this->value_.second);
  }
  iterator end () const {
    return iterator(true);
  }

  ac_value_type value_;
};

template <typename AssociativeContainer>
struct assoc_ctr_iterator_facade {
  typedef AssociativeContainer                            associative_container;
  typedef assoc_ctr_iterator_facade<AssociativeContainer> self_type;

  typedef typename AssociativeContainer::key_type         key_type;
  typedef typename AssociativeContainer::mapped_type      mapped_type;
  typedef typename AssociativeContainer::value_type       value_type;
  typedef typename AssociativeContainer::const_iterator   iterator;

  typedef assoc_ctr_value_facade<AssociativeContainer>    acvf_t;

  typedef boost::variant<acvf_t>                          value_facade_type;

  assoc_ctr_iterator_facade () {}
  assoc_ctr_iterator_facade (iterator const& itr) : iter_(itr) {}

  self_type& operator ++ () {
    ++this->iter_;
    this->value_ = *this->iter_;
    return *this;
  }
  self_type operator ++ (int) {
    self_type cp(this->iter_);
    ++(*this);
    return cp;  
  }

  value_facade_type const& operator * () const {
    return this->value_;
  }
  value_facade_type const* operator -> () const {
    return &this->value_;
  }

  friend bool operator == (self_type const& L, self_type const& R) {
    return L.iter_ == R.iter_;
  }
  friend bool operator != (self_type const& L, self_type const& R) {
    return L.iter_ != R.iter_;
  }

  value_facade_type value_;
  iterator          iter_;
};

} // end vpath namespace

namespace bel {

// list specialization
template <typename T, typename A, typename X>
struct iterator<std::list<T,A>, vpath::xpath<X> > {
  typedef typename std::list<T,A>::const_iterator type;
};
template <typename T, typename A, typename X>
typename iterator<std::list<T,A>, vpath::xpath<X> >::type
begin (std::list<T,A> const& t, vpath::xpath<X>) {
  return t.begin();
}
template <typename T, typename A, typename X>
typename iterator<std::list<T,A>, vpath::xpath<X> >::type
end (std::list<T,A> const& t, vpath::xpath<X>) {
  return t.end();
}
// map specialization
template <typename K, typename V, typename C, typename A, typename X>
struct iterator<std::map<K,V,C,A>, vpath::xpath<X> > {
  typedef xpath::assoc_ctr_iterator_facade<std::map<K,V,C,A> > type;
};
/*template <typename K, typename V, typename C, typename A, typename X>
typename iterator<std::map<K,V,C,A>, vpath::xpath<X> >::type
begin (std::map<K,V,C,A> const& t, vpath::xpath<X>) {
  return t.begin();
}
template <typename K, typename V, typename C, typename A, typename X>
typename iterator<std::map<K,V,C,A>, vpath::xpath<X> >::type
end (std::map<K,V,C,A> const& t, vpath::xpath<X>) {
  return t.end();
}*/
// set specialization
template <typename V, typename C, typename A, typename X>
struct iterator<std::set<V,C,A>, vpath::xpath<X> > {
  typedef typename std::set<V,C,A>::const_iterator type;
};
template <typename V, typename C, typename A, typename X>
typename iterator<std::set<V,C,A>, vpath::xpath<X> >::type
begin (std::set<V,C,A> const& t, vpath::xpath<X>) {
  return t.begin();
}
template <typename V, typename C, typename A, typename X>
typename iterator<std::set<V,C,A>, vpath::xpath<X> >::type
end (std::set<V,C,A> const& t, vpath::xpath<X>) {
  return t.end();
}
// vector specialization
template <typename T, typename A, typename X>
struct iterator<std::vector<T,A>, vpath::xpath<X> > {
  typedef typename std::vector<T,A>::const_iterator type;
};
template <typename T, typename A, typename X>
typename iterator<std::vector<T,A>, vpath::xpath<X> >::type
begin (std::vector<T,A> const& t, vpath::xpath<X>) {
  std::cout << "vector" << std::endl;
  return t.begin();
}
template <typename T, typename A, typename X>
typename iterator<std::vector<T,A>, vpath::xpath<X> >::type
end (std::vector<T,A> const& t, vpath::xpath<X>) {
  return t.end();
}


} // end bel namespace

#endif//VPATH_LIB_BASICS
