#include <xpath/vpath.hpp>
#include <xpath/path.hpp>
#include <xpath/query.hpp>
#include <jsonpp.hpp>
#include <xpath/builtin.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>

template <typename Xpath>
std::string name (JSONpp::nil, Xpath) {
  return "nil";
}
template <typename Xpath>
std::string name (JSONpp::json_v, Xpath) {
  return "json";
}

int main (int argc, char *argv[]) {

  typedef bel::iterator<bool,vpath::xpath<void> >::type biter;

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
