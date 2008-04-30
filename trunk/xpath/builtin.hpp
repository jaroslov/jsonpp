// vpath
#include "concepts.hpp"

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

} // end vpath namespace

#endif//VPATH_LIB_BASICS
