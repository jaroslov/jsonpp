#include "jsonpp.hpp"
#include <xpgtl/xpath.hpp>

#ifndef JSONPP_XPGTL_INTERFACE
#define JSONPP_XPGTL_INTERFACE

//
// augment the "tag" overload set for nil and json_v
namespace xpgtl {
  std::string tag (JSONpp::nil, xpath<JSONpp::json_v>) {
    return "nil";
  }
  std::string tag (bool, xpath<JSONpp::json_v>) {
    return "bool";
  }
  std::string tag (std::wstring, xpath<JSONpp::json_v>) {
    return "string";
  }
  std::string tag (std::vector<JSONpp::json_v>, xpath<JSONpp::json_v>) {
    return "array";
  }
  std::string tag (std::map<std::wstring,JSONpp::json_v>, xpath<JSONpp::json_v>) {
    return "object";
  }
  template <typename X>
  struct has_children<JSONpp::json_v, xpath<X> > : boost::mpl::true_ {
    typedef boost::variant<
      typename JSONpp::json_gen::value_t const*,
      typename JSONpp::json_gen::string_t const*,
      typename JSONpp::json_gen::number_t const*,
      typename JSONpp::json_gen::object_t const*,
      typename JSONpp::json_gen::array_t const*,
      typename JSONpp::json_gen::bool_t const*,
      typename JSONpp::json_gen::null_t const*> type;
  };
}

#endif//JSONPP_XPGTL_INTERFACE
