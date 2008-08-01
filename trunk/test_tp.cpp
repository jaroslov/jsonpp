#include <iostream>
#include <fstream>
#include <locale>
#include <sstream>
#include <json/jsonpp.hpp>
#include <treepath/simple_xpath.hpp>
#include <treepath/query.hpp>

namespace treepath {

	template <typename Tag>
	struct node_traits<JSONpp::json_v, Tag> {
		typedef int node_variant;
		typedef std::wstring node_test_type;
	};

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
			
			treepath::query(json, path, treepath::treepath);
			
      std::cout << std::endl;
    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
	}

	return 0;
}
