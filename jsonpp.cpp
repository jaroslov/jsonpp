#include "jsonpp.hpp"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>

int main (int argc, char *argv[]) {

  if (argc < 1)
    return 1;

  for (++argv; argc > 0; --argc, ++argv) {
    std::cout << *argv << std::endl;
    try {
      std::ifstream ifstr(*argv);
      std::istream_iterator<char> ctr(ifstr);
      std::istream_iterator<char> cnd;
      JSONpp::json_v json = JSONpp::parse(ctr, cnd);
      JSONpp::print(json);
      std::cout << std::endl;
    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
  }

  return 0;
}

