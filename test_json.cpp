//#define DEBUG_JSON
#include <json/jsonpp.hpp>

#include <iostream>
#include <fstream>
#include <locale>

// begin testing unicode conversion
/*
*/
enum json_utf_encoding {
  UTF_32BE  = 'B',
  UTF_16BE  = 'b',
  UTF_32LE  = 'L',
  UTF_16LE  = 'l',
  UTF_8     = '8',
  error     = 'e',
  unknown   = 'u',
};
json_utf_encoding encoding (std::string const& fname) {
  // 00 00 00 xx  UTF-32BE
  // 00 xx 00 xx  UTF-16BE
  // xx 00 00 00  UTF-32LE
  // xx 00 xx 00  UTF-16LE
  // xx xx xx xx  UTF-8
  std::ifstream look(fname.c_str(), std::ios::binary|std::ios::in);
  char block[5];
  look.read(block, 4);
  look.close();
  for (std::size_t i=0; i<4; ++i)
    std::cout << (int)block[i] << " " << std::flush;
  std::cout << std::endl;
  return unknown;
}

int main (int argc, char *argv[]) {

  

  std::locale loc("C");
  std::wcout.imbue(loc);

  if (argc < 1)
    return 1;

  for (++argv; argc > 0; --argc, ++argv) {
    std::cout << *argv << std::endl;
    try {
      std::cout << (char)encoding(std::string(*argv)) << std::endl;;

      std::wifstream wifstr(*argv);
      wifstr.imbue(loc);
      wifstr >> std::noskipws;
      std::istream_iterator<wchar_t,wchar_t> ctr(wifstr);
      std::istream_iterator<wchar_t,wchar_t> cnd;
      JSONpp::json_v json = JSONpp::parse(ctr, cnd);
      JSONpp::json_gen::string_t str = JSONpp::to_string(json, true);
      //std::wcout << str << std::endl;
    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
  }

  return 0;
}

