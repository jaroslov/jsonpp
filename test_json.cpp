//#define DEBUG_JSON
#include <json/jsonpp.hpp>

#include <iostream>
#include <fstream>
#include <locale>

typedef std::basic_string<std::size_t> jstring;
// begin testing unicode conversion
enum json_utf_encoding {
  error     = 0,
  UTF_32BE  = 1,
  UTF_16BE  = 5,
  UTF_32LE  = 8,
  UTF_16LE  = 10,
  UTF_8     = 15,
  unknown   = 32,
};
json_utf_encoding encoding (std::string const& fname) {
  // 00 00 00 xx  UTF-32BE => 1
  // 00 xx 00 xx  UTF-16BE => 5
  // xx 00 00 00  UTF-32LE => 8
  // xx 00 xx 00  UTF-16LE => 10
  // xx xx xx xx  UTF-8    => 15
  std::ifstream look(fname.c_str(), std::ios::binary|std::ios::in);
  if (not look) return error;
  char block[5];
  look.read(block, 4);
  if (not look) return unknown;
  look.close();
  int encode_  = (0 != block[0] ? 8 : 0)
               | (0 != block[1] ? 4 : 0)
               | (0 != block[2] ? 2 : 0)
               | (0 != block[3] ? 1 : 0);
  return (json_utf_encoding)encode_;
}

jstring transcode (char* file, std::size_t length, json_utf_encoding jue) {

  std::size_t utf8_len = (3*length)/2+1;
  char * buffer = (char*)calloc(utf8_len,1);
  char * bufstart = buffer;
  const char * utf_32be = "UTF-32BE";
  const char * utf_16be = "UTF-16BE";
  const char * utf_32le = "UTF-32LE";
  const char * utf_16le = "UTF-16LE";
  const char * utf_8    = "UTF8";
  const char * from;
  switch (jue) {
  case UTF_32BE: from = utf_32be; break;
  case UTF_16BE: from = utf_16be; break;
  case UTF_32LE: from = utf_32le; break;
  case UTF_16LE: from = utf_16le; break;
  case UTF_8:
  default:  from = utf_8; break; // why not?
  }
  iconv_t cd = iconv_open("UTF-16BE", from);
  std::size_t inbytesleft = length, outbytesleft = utf8_len;
  std::size_t result = iconv(cd,
                          &file, &inbytesleft,
                          &buffer, &outbytesleft);
  if (iconv_close(cd))
    throw std::runtime_error("iconv somehow failed");
  const std::size_t conv_length = utf8_len - outbytesleft;
  jstring rawstr;
  for (std::size_t i=0; i<conv_length/2; i+=2) {
    const std::size_t hi = (unsigned char)bufstart[i];
    const std::size_t lo = (unsigned char)bufstart[i+1];
    // now, C is the codepoint
    if (0 == hi)
      rawstr += (std::size_t)lo;
    else
      rawstr += (hi << 16) & lo;
  }
  free(bufstart);
  return rawstr;
}
jstring transcode_file (std::string const& fname) {
  std::ifstream wif(fname.c_str(), std::ios::binary|std::ios::in);
  const std::size_t position = wif.tellg();
  wif.seekg(0, std::ios::end);
  const signed long length = (signed long)wif.tellg() - (signed long)position;
  wif.seekg(position);
  char buffer[length+4];
  wif.read(buffer, length);
  wif.close();
  return transcode(buffer, length, encoding(fname));
}

int main (int argc, char *argv[]) {

  if (argc < 1)
    return 1;

  for (++argv; argc > 0; --argc, ++argv) {
    std::cout << *argv << std::endl;
    try {
      transcode_file(std::string(*argv));

      {
        std::wifstream wifstr(*argv);
        wifstr >> std::noskipws;
        std::istream_iterator<wchar_t,wchar_t> ctr(wifstr);
        std::istream_iterator<wchar_t,wchar_t> cnd;
        while (ctr != cnd) {
          std::cout << (int)(unsigned char)*ctr << " ";
          ++ctr;
        }
        std::cout << std::endl;
      }

      std::wifstream wifstr(*argv);
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

