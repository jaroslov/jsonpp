#include "jsonpp.hpp"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>

int main (int argc, char *argv[]) {

  typedef JSONpp::make_json_value<> mjv_t;

  if (argc < 1)
    return 1;

  for (++argv; argc > 0; --argc, ++argv) {
    std::cout << *argv << std::endl;
    std::ifstream ifstr(*argv);
    std::wstring file = L"";
    char buffer[1025];
    std::memset(buffer, 1025, 0);
    while (ifstr.good()) {
      ifstr.read(buffer, 1024);
      file += std::wstring(buffer,buffer+ifstr.gcount());
    }
    JSONpp::push_parser<mjv_t> parser;
    mjv_t::type value = parser(file);
    JSONpp::variant_json_printer<mjv_t> Pr;
    boost::apply_visitor(Pr, value);
    std::cout << std::endl;
    file = L"";
    ifstr.close();
  }

  return 0;
}

