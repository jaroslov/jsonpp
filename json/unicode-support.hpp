// ICONV
#include <iconv.h>

#ifndef JSONPP_UNICODE_SUPPORT
#define JSONPP_UNICODE_SUPPORT

namespace JSONpp {

// JSON is a little inconsistent in its representation of strings.
// It says (rfc4627) that the encodings in the following table are legal:
//
//    00 00 00 xx  UTF-32BE => 1
//    00 xx 00 xx  UTF-16BE => 5
//    xx 00 00 00  UTF-32LE => 8
//    xx 00 xx 00  UTF-16LE => 10
//    xx xx xx xx  UTF-8    => 15
//
// And that the "UTF-8" encoding is the default encoding. However, its
// `escape' mechanism uses UTF-16BE, that is, when representing unicode
// character points as a stream of the form "\uXXXX" it uses the UTF-16BE
// encoding scheme.
//
// Our JSON parser will thus always use an internal representation of the
// string as ASCII, where characters outside the range [32,127] will be
// represented as code-points of the form "\uXXXX" (plus any appropriate
// surrogates).
//
// This means that we must provide a convenient way for users to give us
// a string, and for us to convert it into canonical form, and for us to be
// able to recognize a string in canonical form.

struct jstring : std::string {
  jstring () : std::string() {}
};

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
