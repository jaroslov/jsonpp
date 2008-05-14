// ICONV
#include <iconv.h>

#ifndef JSONPP_UNICODE_SUPPORT
#define JSONPP_UNICODE_SUPPORT

namespace JSONpp {

template <typename Iterator>
std::string transcode (Iterator const& itr, Iterator const& ind) {
  typedef typename Iterator::value_type value_type;
  enum encoding_e {
    error     = 0,
    UTF_32BE  = 1,
    UTF_16BE  = 5,
    UTF_32LE  = 8,
    UTF_16LE  = 10,
    UTF_8     = 15,
    unknown   = 32,
  } encoding = unknown;
}

}

#endif//JSONPP_UNICODE_SUPPORT
