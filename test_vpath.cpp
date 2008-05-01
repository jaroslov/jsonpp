//#define XPGTL_DEBUG
#include <xpgtl/xpath.hpp>
#include <json/jsonpp.hpp>
#include <json/xpath.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>

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
      xpgtl::print_result_set(xpgtl::query(path, json));
      std::cout << std::endl;

      std::set<const std::wstring*> qset
        = xpgtl::query(path, json, xpgtl::as<std::wstring>());
      std::set<const std::wstring*>::iterator first, last;
      boost::tie(first,last)=bel::sequence(qset);
      std::wcout << L"Typeful query..." << std::endl << L"[";
      if (first != last) {
        std::wcout << L"\"" << **first << L"\""; // holds pointers!
        ++first;
      }
      for ( ; first != last; ++first) {
        std::wcout << L", ";
        std::wcout << L"\"" << **first << L"\"";
      }
      std::wcout << L"]" << std::endl;
    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
  }

  return 0;
}
