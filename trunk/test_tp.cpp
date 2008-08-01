#include <iostream>
#include <fstream>
#include <locale>
#include <sstream>
#include <json/jsonpp.hpp>
#include <treepath/simple_xpath.hpp>
#include <treepath/query.hpp>

namespace treepath {

	template <typename Tag>
	struct node_traits<json::json_v, Tag> {
		typedef int node_variant;
		typedef std::wstring node_test_type;
	};

}

int main (int argc, char *argv[]) {

	if (argc < 3)
		return 0;

	treepath::path<std::wstring> path = treepath::simple_xpath::parse(argv[1]);

  std::locale loc("");
  std::wcout.imbue(loc);

	std::cout << std::string(20, '=') << std::endl;
	std::cout << "Path: " << path << std::endl << std::endl << std::endl;

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

			treepath::query(json, path);

      std::cout << std::endl;
    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
  }

	return 0;
}
