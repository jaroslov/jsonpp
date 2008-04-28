#include <jsonpp.hpp>

#include <iostream>
#include <fstream>
#include <locale>

int main (int argc, char *argv[]) {

  std::locale loc("");
  std::wcout.imbue(loc);

  if (argc < 1)
    return 1;

  for (++argv; argc > 0; --argc, ++argv) {
    std::cout << *argv << std::endl;
    try {
      std::wifstream wifstr(*argv);
      wifstr.imbue(loc);
      wifstr >> std::noskipws;
      std::istream_iterator<wchar_t,wchar_t> ctr(wifstr);
      std::istream_iterator<wchar_t,wchar_t> cnd;
      JSONpp::json_v json = JSONpp::parse(ctr, cnd);
      JSONpp::json_gen::string_t str = JSONpp::to_string(json);
      std::wcout << str << std::endl;
    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
  }

  return 0;
}

