//#define DEBUG_JSON
#include <json/jsonpp.hpp>

#include <iostream>
#include <fstream>
#include <locale>

typedef std::basic_string<std::size_t> jstring;
// begin testing unicode conversion
/*json_utf_encoding encoding (std::string const& fname) {
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
}*/

template <typename Iterator, std::size_t BUFLEN=512>
struct transcode_iterator {
  enum encoding_e {
    error     = 0,
    UTF_32BE  = 1,
    UTF_16BE  = 5,
    UTF_32LE  = 8,
    UTF_16LE  = 10,
    UTF_8     = 15,
    unknown   = 32,
  };

  typedef Iterator iterator_type;
  typedef transcode_iterator<iterator_type> self_type;
  typedef typename Iterator::value_type     ivalue_type;
  typedef char                              value_type;
  typedef char const&                       reference_type;
  typedef char const*                       pointer_type;
  typedef reference_type                    result_type;
  typedef transcode_iterator<iterator_type> postincrement_result;
  typedef signed long                       difference_type;

  transcode_iterator ()
    : packed(false), encoding(unknown)
    , cd((iconv_t)-1), offset(BUFLEN-1)
    , first(), last() {}
  transcode_iterator (iterator_type const& F, iterator_type const& L)
    : packed(false)
    , encoding(unknown), cd((iconv_t)-1), offset(BUFLEN-1)
    , first(F), last(L) {
    this->normalize();
  }
  transcode_iterator (self_type const& st) {
    this->copy(st);
  }
  ~ transcode_iterator () {
    this->close();
  }
  self_type& operator = (self_type const& st) {
    this->copy(st); return *this;
  }

  void copy (self_type const& st) {
    this->encoding = st.encoding;
    this->packed = st.packed;
    this->offset = st.offset;
    this->first = st.first;
    this->last = st.last;
    this->cd = (iconv_t)-1;
    this->get_iconv();
    std::memcpy(this->buffer, st.buffer, BUFLEN);
  }

  void close () {
    if ((iconv_t)-1 != this->cd)
      iconv_close(this->cd);
  }

  void normalize () {
    if (this->first == this->last) return;
    // okay, sizeof(value_type) > sizeof(char), which
    // means that value_type could contain multiple "chars"
    // or it could just be an inefficient packed mechanism
    //
    // either way, we need to grab a bunch of values
    value_type vbuffer[BUFLEN];
    this->offset = 0;
    while ((this->first != this->last) and (this->offset < BUFLEN)) {
      vbuffer[this->offset] = 0;
      vbuffer[this->offset] = *this->first;
      ++this->first, ++this->offset;
    }
    // so, if we don't know the encoding, we also don't know the packed
    if (unknown == this->encoding) {
      // if the value of the first value is < 256, then we have
      // "unpacked storage" and we set packed to "false", otherwise
      // the value of first value is > 256, and we have "packed storage"
      //
      // the strange construct suppresses warnings when
      //    2^[sizeof(vbuffer)*CHAR_BITS] <= 256
      this->packed = not ((vbuffer[0] >> 8) > 0)/*not (vbuffer[0] < 256)*/;
    }
    // we need to get an array of chars whose length depends upon packed;
    // then we populate that array according to the packed format
    char *rbuffer = 0;
    if (not this->packed) {
      rbuffer = (char*)calloc(BUFLEN, 1);
      for (std::size_t i=0; i<BUFLEN; ++i)
        rbuffer[i] = (char)vbuffer[i]; // more common AND slower
    } else {
      rbuffer = (char*)calloc(BUFLEN*sizeof(value_type), 1);
      char *rcb = reinterpret_cast<char*>(vbuffer);
      std::memcpy(rbuffer, rcb, BUFLEN*sizeof(value_type));
    }
    // now, we have (at least) the first four values, but we may still not
    // know the encoding; so we're going to figure it out!
    if (unknown == this->encoding) {
      this->encoding  = (encoding_e)((0 != rbuffer[0] ? 8 : 0)
                                   | (0 != rbuffer[1] ? 4 : 0)
                                   | (0 != rbuffer[2] ? 2 : 0)
                                   | (0 != rbuffer[3] ? 1 : 0));
      this->get_iconv(); // get the iconv type
    }
    // finally, we convert our rbuffer characters into the appropriate
    // local chars
    free(rbuffer);
  }

  void get_iconv () {
    this->close();
    const char * utf_32be = "UTF-32BE";
    const char * utf_16be = "UTF-16BE";
    const char * utf_32le = "UTF-32LE";
    const char * utf_16le = "UTF-16LE";
    const char * utf_8    = "UTF8";
    const char * from;
    switch (this->encoding) {
    case UTF_32BE: from = utf_32be; break;
    case UTF_16BE: from = utf_16be; break;
    case UTF_32LE: from = utf_32le; break;
    case UTF_16LE: from = utf_16le; break;
    case UTF_8:
    default:  from = utf_8; break; // why not?
    }
    iconv_t cd = iconv_open("UTF-16BE", from);
  }

  pointer_type operator -> () const {
    return this->buffer+this->offet;
  }
  reference_type operator * () const {
    return this->buffer[this->offset];
  }
  self_type& operator ++ () {
    ++this->offset;
    this->normalize();
    return *this;
  }
  postincrement_result operator ++ (int) {
    postincrement_result cp(*this); ++*this;
    return cp;
  }

  friend bool operator == (self_type const& L, self_type const& R) {
    return (L.offset == R.offset)
      and (L.first == R.first)
      and (L.last == R.last);
  }
  friend bool operator != (self_type const& L, self_type const& R) {
    return (L.offset != R.offset)
      or (L.first != R.first)
      or (L.last != R.last);
  }

protected:
  bool packed;
  encoding_e encoding;
  iconv_t cd;
  std::size_t offset;
  iterator_type first, last; 
  char buffer[BUFLEN];
};
template <typename Iterator>
transcode_iterator<Iterator>
build_transcode_iterator (Iterator const& F, Iterator const& L) {
  return transcode_iterator<Iterator>(F, L);
}


int main (int argc, char *argv[]) {

  if (argc < 1)
    return 1;

  for (++argv; argc > 0; --argc, ++argv) {
    std::cout << *argv << std::endl;
    try {    
      std::wifstream wifstr(*argv);
      wifstr >> std::noskipws;
      std::istream_iterator<wchar_t,wchar_t> ctr(wifstr);
      std::istream_iterator<wchar_t,wchar_t> cnd;
      JSONpp::json_v json = JSONpp::parse(ctr, cnd);
      JSONpp::json_gen::string_t str = JSONpp::to_string(json, true);
      std::wcout << str << std::endl;
    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
  }

  return 0;
}

