//#define XPGTL_DEBUG
#include <xpgtl/xpath.hpp>
#include <json/jsonpp.hpp>
#include <json/xpath.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>

namespace xpgtl {

template <typename String, typename R>
void Query (xpgtl::path<String> const& path, R const& r) {
  std::cout << "HERE" << std::endl;
}

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
      xpgtl::Query(path, json);
      std::cout << std::endl;

    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
  }

  return 0;
}
