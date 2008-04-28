#include "vpath.hpp"
#include "assoc_ctr.hpp"
#include <begin-end.hpp>
#include <list>
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
template <typename K, typename V, typename C, typename A>
struct recursive<std::map<K,V,C,A> > : boost::mpl::true_ {};
template <typename K, typename C, typename A>
struct recursive<std::set<K,C,A> > : boost::mpl::true_ {};
template <typename T, typename A>
struct recursive<std::vector<T,A> > : boost::mpl::true_ {};

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
// set specialization
template <typename K, typename V, typename C, typename A, typename X>
struct iterator<std::map<K,V,C,A>, vpath::xpath<X> > {
  typedef vpath::assoc_ctr::value_iterator_facade<std::map<K,V,C,A> > type;
};
template <typename K, typename V, typename C, typename A, typename X>
typename iterator<std::map<K,V,C,A>, vpath::xpath<X> >::type
begin (std::map<K,V,C,A> const& t, vpath::xpath<X>) {
  typedef typename iterator<std::map<K,V,C,A>, vpath::xpath<X> >::type iter;
  return iter(t.begin());
}
template <typename K, typename V, typename C, typename A, typename X>
typename iterator<std::map<K,V,C,A>, vpath::xpath<X> >::type
end (std::map<K,V,C,A> const& t, vpath::xpath<X>) {
  typedef typename iterator<std::map<K,V,C,A>, vpath::xpath<X> >::type iter;
  return iter(t.end());
}
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
