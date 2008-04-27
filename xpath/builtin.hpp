#include "vpath.hpp"
#include <begin-end.hpp>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <boost/variant.hpp>

#ifndef VPATH_LIB_BASICS
#define VPATH_LIB_BASICS

namespace vpath {

template <typename X> std::string tag (bool, xpath<X>) { return "bool"; }
template <typename X> std::string tag (char, xpath<X>) { return "char"; }
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

struct no_iterator {
  struct nil {};
  typedef boost::variant<nil> zilch_t;
  typedef zilch_t value_type;

  friend bool operator == (no_iterator const& L, no_iterator const& R) {
    return true;
  }
  friend bool operator != (no_iterator const& L, no_iterator const& R) {
    return false;
  }
  no_iterator operator ++ () { return *this; }
  no_iterator operator ++ (int) { return *this; }
  zilch_t const& operator * () const { return this->nada; }
  zilch_t const* operator -> () const { return &this->nada; }

  zilch_t nada;
};

} // end vpath namespace

namespace bel {

template <typename T, typename X>
struct iterator<T, vpath::xpath<X> > {
  typedef vpath::no_iterator type;
};

template <typename T, typename X>
typename iterator<T, vpath::xpath<X> >::type
begin (T const& t, vpath::xpath<X>) {
  return typename iterator<T, vpath::xpath<X> >::type();
}
template <typename T, typename X>
typename iterator<T, vpath::xpath<X> >::type
end (T const& t, vpath::xpath<X>) {
  return typename iterator<T, vpath::xpath<X> >::type();
}
template <typename T, typename X>
std::pair<typename iterator<T, vpath::xpath<X> >::type,
  typename iterator<T, vpath::xpath<X> >::type>
sequence (T const& t, vpath::xpath<X> x) {
  return std::make_pair(bel::begin(t,x),bel::end(t,x));
}

} // end bel namespace

#endif//VPATH_LIB_BASICS
