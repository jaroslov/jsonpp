#include <iostream>
#include <fstream>
#include <locale>
#include <sstream>
#include <json/jsonpp.hpp>
#include <treepath/simple_xpath.hpp>
#include <treepath/query.hpp>
#include <utility/iterator.hpp>

namespace bel {

	template <>
	struct iterator<JSONpp::json_gen::array_t, treepath::treepath_<JSONpp::json_v> > {
		typedef JSONpp::json_gen::array_t::const_iterator type;
	};

}

namespace treepath {

	template <>
	struct node_traits<JSONpp::json_v, treepath_<JSONpp::json_v> > {
		typedef boost::variant<
      JSONpp::json_gen::value_t const*,  // store values (never used)
      JSONpp::json_gen::string_t const*, // store strings
      JSONpp::json_gen::number_t const*, // store "numbers" (double)
      JSONpp::json_gen::object_t const*, // store objects
      JSONpp::json_gen::array_t const*,  // store arrays
      JSONpp::json_gen::bool_t const*,   // store booleans
      JSONpp::json_gen::null_t const*   // store Null> node_variant;
			> node_variant;

		typedef treepath_<JSONpp::json_v> node_traits_tag;
		typedef std::wstring node_test_type;
	};

	template <>
	struct has_children<JSONpp::json_gen::array_t, treepath_<JSONpp::json_v> > : boost::mpl::true_ {};

  std::wstring node_test (JSONpp::nil, treepath_<JSONpp::json_v>) {
    return L"nil";
  }
  std::wstring node_test (bool, treepath_<JSONpp::json_v>) {
    return L"bool";
  }
  std::wstring node_test (std::string, treepath_<JSONpp::json_v>) {
    return L"string";
  }
  std::wstring node_test (std::vector<JSONpp::json_v>, treepath_<JSONpp::json_v>) {
    return L"array";
  }
  std::wstring node_test (std::map<std::string,JSONpp::json_v>, treepath_<JSONpp::json_v>) {
    return L"object";
  }
  std::wstring node_test (JSONpp::json_v, treepath_<JSONpp::json_v>) {
    return L"json";
  }

}

int main (int argc, char *argv[]) {

	if (argc < 3)
		return 0;

	std::wstring pathwstr(argv[1], argv[1]+std::strlen(argv[1]));

	treepath::path<std::wstring> path = treepath::simple_xpath::parse(pathwstr);

  std::locale loc("");
  std::wcout.imbue(loc);

	std::wcout << std::wstring(20, '=') << std::endl;
	std::wcout << path << std::endl << std::endl << std::endl;

	argv += 2;

  for (std::size_t i=2; i<argc; ++i, ++argv) {
    std::cout << *argv << std::endl;
    try {
      std::wifstream wifstr(*argv);
      wifstr.imbue(loc);
      wifstr >> std::noskipws;
      std::istream_iterator<wchar_t,wchar_t> ctr(wifstr);
      std::istream_iterator<wchar_t,wchar_t> cnd;
      JSONpp::json_v json = JSONpp::parse(ctr, cnd);

			treepath::query(json, path, treepath::treepath_<JSONpp::json_v>(), treepath::unwrap);
			
      std::cout << std::endl;
    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
	}

	return 0;
}
