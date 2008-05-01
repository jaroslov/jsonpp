#include "jsonpp.hpp"
#include <xpgtl/xpath.hpp>
#include <rdstl/rdstl.hpp>

#ifndef JSONPP_XPGTL_INTERFACE
#define JSONPP_XPGTL_INTERFACE

//
// augment the "tag" overload set for nil and json_v
namespace JSONpp {
  std::string tag (JSONpp::nil, xpgtl::xpath<JSONpp::json_v>) {
    return "nil";
  }
  std::string tag (bool, xpgtl::xpath<JSONpp::json_v>) {
    return "bool";
  }
  std::string tag (std::wstring, xpgtl::xpath<JSONpp::json_v>) {
    return "string";
  }
  std::string tag (std::vector<JSONpp::json_v>, xpgtl::xpath<JSONpp::json_v>) {
    return "array";
  }
  std::string tag (std::map<std::wstring,JSONpp::json_v>,xpgtl:: xpath<JSONpp::json_v>) {
    return "object";
  }
} // end json namespace
namespace rdstl {
  template <typename X>
  struct has_children<JSONpp::json_v, xpgtl::xpath<X> > : boost::mpl::true_ {
    typedef boost::variant<
      typename JSONpp::json_gen::value_t const*,
      typename JSONpp::json_gen::string_t const*,
      typename JSONpp::json_gen::number_t const*,
      typename JSONpp::json_gen::object_t const*,
      typename JSONpp::json_gen::array_t const*,
      typename JSONpp::json_gen::bool_t const*,
      typename JSONpp::json_gen::null_t const*> type;
  };
} // end rdstl namespace

#endif//JSONPP_XPGTL_INTERFACE
