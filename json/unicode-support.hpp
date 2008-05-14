// ICONV
#include <iconv.h>

#ifndef JSONPP_UNICODE_SUPPORT
#define JSONPP_UNICODE_SUPPORT

namespace JSONpp {

template <typename Iterator>
std::string transcode (Iterator itr, Iterator const& ind) {
  if (itr == ind) return std::string();

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
  value_type block[4] = {0, 0, 0, 0};
  value_type fvalue = *itr;

  // first, we need to determine if the values are packed
  bool packed = not ((fvalue>>8) > 0)/*not (fvalue > 255)*/;

  std::size_t offset = 1;
  while ((itr != ind) and (offset < 4))
    block[offset] = *itr,
    ++itr,
    ++offset;

  return std::string();
}

}

#endif//JSONPP_UNICODE_SUPPORT
