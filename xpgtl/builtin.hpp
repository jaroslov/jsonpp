#include "concepts.hpp"
#include <rdstl/rdstl.hpp>

#ifndef XPGTL_LIB_BASICS
#define XPGTL_LIB_BASICS

namespace rdstl {

// rsdtl support for tagging
template <typename X> struct has_tag<bool,xpgtl::xpath<X> > : xpgtl::builtin_tag_base<bool,X> {};
template <typename X> struct has_tag<unsigned char,xpgtl::xpath<X> > : xpgtl::builtin_tag_base<unsigned char,X> {};
template <typename X> struct has_tag<signed char,xpgtl::xpath<X> > : xpgtl::builtin_tag_base<signed char,X> {};
template <typename X> struct has_tag<wchar_t,xpgtl::xpath<X> > : xpgtl::builtin_tag_base<wchar_t,X> {};
template <typename X> struct has_tag<signed short,xpgtl::xpath<X> > : xpgtl::builtin_tag_base<signed short,X> {};
template <typename X> struct has_tag<unsigned short,xpgtl::xpath<X> > : xpgtl::builtin_tag_base<unsigned short,X> {};
template <typename X> struct has_tag<signed int,xpgtl::xpath<X> > : xpgtl::builtin_tag_base<signed int,X> {};
template <typename X> struct has_tag<unsigned int,xpgtl::xpath<X> > : xpgtl::builtin_tag_base<unsigned int,X> {};
template <typename X> struct has_tag<signed long,xpgtl::xpath<X> > : xpgtl::builtin_tag_base<signed long,X> {};
template <typename X> struct has_tag<unsigned long,xpgtl::xpath<X> > : xpgtl::builtin_tag_base<unsigned long,X> {};
template <typename X> struct has_tag<float,xpgtl::xpath<X> > : xpgtl::builtin_tag_base<float,X> {};
template <typename X> struct has_tag<double,xpgtl::xpath<X> > : xpgtl::builtin_tag_base<double,X> {};

}

namespace xpgtl {

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

} // end xpgtl namespace

#endif//XPGTL_LIB_BASICS
