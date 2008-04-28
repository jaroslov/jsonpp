#include <xpath/vpath.hpp>
#include <xpath/path.hpp>
#include <xpath/query.hpp>
#include <jsonpp.hpp>
#include <xpath/builtin.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>

//
// augment the "tag" overload set for nil and json_v
namespace vpath {
  std::string tag (JSONpp::nil, vpath::xpath<JSONpp::json_v>) {
    return "nil";
  }
  std::string tag (bool, vpath::xpath<JSONpp::json_v>) {
    return "bool";
  }
  std::string tag (std::wstring, vpath::xpath<JSONpp::json_v>) {
    return "string";
  }
  std::string tag (std::vector<JSONpp::json_v>, vpath::xpath<JSONpp::json_v>) {
    return "array";
  }
  std::string tag (std::map<std::wstring,JSONpp::json_v>, vpath::xpath<JSONpp::json_v>) {
    return "object";
  }
  std::string tag (JSONpp::json_v, vpath::xpath<JSONpp::json_v>) {
    return "json";
  }
}

int main (int argc, char *argv[]) {

  std::locale loc("");
  std::wcout.imbue(loc);

  std::string input;
  std::istream_iterator<char> ctr(std::cin), cnd;
  std::copy(ctr, cnd, std::back_inserter(input));
  vpath::path path = vpath::parser(input);
  std::cout << path << std::endl << std::endl;

  for (++argv; argc > 0; --argc, ++argv) {
    std::cout << *argv << std::endl;
    try {
      std::wifstream wifstr(*argv);
      wifstr.imbue(loc);
      wifstr >> std::noskipws;
      std::istream_iterator<wchar_t,wchar_t> ctr(wifstr);
      std::istream_iterator<wchar_t,wchar_t> cnd;
      JSONpp::json_v json = JSONpp::parse(ctr, cnd);
      vpath::query(path, json);
    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
  }

  return 0;
}
