#define XPGTL_DEBUG
#include <xpgtl/xpath.hpp>
#include <json/jsonpp.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>

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

int main (int argc, char *argv[]) {

  std::locale loc("");
  std::wcout.imbue(loc);

  std::string input;
  std::istream_iterator<char> ctr(std::cin), cnd;
  std::copy(ctr, cnd, std::back_inserter(input));
  xpgtl::path<std::string> path = xpgtl::path<std::string>(input);
  std::cout << xpgtl::abbreviate << path << std::endl;
  std::cout << xpgtl::long_form << path << std::endl;
  std::cout << std::endl;

  for (++argv; argc > 0; --argc, ++argv) {
    std::cout << *argv << std::endl;
    try {
      std::wifstream wifstr(*argv);
      wifstr.imbue(loc);
      wifstr >> std::noskipws;
      std::istream_iterator<wchar_t,wchar_t> ctr(wifstr);
      std::istream_iterator<wchar_t,wchar_t> cnd;
      JSONpp::json_v json = JSONpp::parse(ctr, cnd);
      xpgtl::print_result_set(xpgtl::query(path, json));
      std::cout << std::endl;

      std::set<const std::wstring*> qset = xpgtl::query(path, json, (std::wstring*)0);
      std::set<const std::wstring*>::iterator first, last;
      boost::tie(first,last)=bel::sequence(qset);
      std::wcout << L"Typeful query..." << std::endl << L"[";
      if (first != last) {
        std::wcout << L"\"" << **first << L"\""; // holds pointers!
        ++first;
      }
      for ( ; first != last; ++first) {
        std::wcout << L", ";
        std::wcout << L"\"" << **first << L"\"";
      }
      std::wcout << L"]" << std::endl;
    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
  }

  return 0;
}
