// BEL library
#include <bel/begin-end.hpp>
// boost
#include <boost/variant.hpp>
#include <boost/variant/recursive_variant.hpp>
// STL
#include <exception>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
// iconv
#include <iconv.h>

#ifndef JSON_PARSER
#define JSON_PARSER

namespace JSONpp {

// converts an arbitrary type that satisfies
// the string interface for the STL, and whose
// value_type is static_cast'able to "char"
// into a std::string (for printing)
template <typename String>
std::string to_ascii_string (String const& str) {
  typedef typename String::const_iterator citer;
  citer first = bel::begin(str), last = bel::end(str);
  std::string rstr;
#ifdef DEBUG_JSON
  std::cout << std::endl;
#endif
  while (first != last) {
#ifdef DEBUG_JSON
    std::cout << (wchar_t)*first << std::endl;
    std::wcout << (wchar_t)*first << std::endl;
#endif
    const char C = static_cast<char>(*first);
    if ((31 < C) and (C < 127))
      rstr += C;
    else {
      rstr += "\\u?";
    }
    ++first;
  }
  return rstr;
}

//=== [JSTRING (UNICODE SUPPORT)] ===
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

struct jstring : public std::string {
  jstring () : std::string() {}
  ~ jstring () {}
  template <typename T>
  jstring (T const* ttr, T const* tnd) {
    this->load_ptr(ttr, tnd);
  }
  template <typename Iter>
  jstring (Iter itr, Iter const& ind) {
    this->load_iter(itr, ind);
  }
  template <typename T>
  void load_ptr (T const* ttr, T const* tnd) {
    if (ttr == tnd) return;
    const std::size_t t_length = tnd - ttr;
    // figure out if T is packed
    // if T is "packed" (more than one character per element)
    // then "unpack" it by copying its memory into a buffer,
    // otherwise, copy the data into the buffer
    bool packed = (*ttr>>8) > 0;// not (ttr[0] > 255);
    char *buffer = 0;
    if (packed) {
      buffer = (char*)calloc(t_length, sizeof(T));
      std::memcpy(buffer, reinterpret_cast<const char*>(ttr), t_length);
    } else {
      buffer = (char*)calloc(t_length, 1);
      char *tbuf = buffer;
      while (ttr != tnd)
        *tbuf = (char)*ttr,
        ++ttr, ++tbuf;
    }
    this->load_ptr(buffer, buffer+t_length);
    free(buffer);
  }
  template <typename T>
  void load_iter (T itr, T const& ind) {
    typedef typename T::value_type value_type;
    if (itr == ind) return;
    const std::size_t t_length = ind - itr;
    value_type buffer[t_length];
    value_type *tbuf = buffer;
    while (itr != ind)
      *tbuf = *itr,
      ++tbuf, ++itr;
    this->load_ptr(buffer, buffer+t_length);
  }
  void load_ptr (char const* ttr, char const* tnd) {
    if (ttr == tnd) return;
    const std::size_t t_length = tnd - ttr;
    const char * utf_32be = "UTF-32BE";
    const char * utf_16be = "UTF-16BE";
    const char * utf_32le = "UTF-32LE";
    const char * utf_16le = "UTF-16LE";
    const char * utf_8    = "UTF8";
    const char * from;
    if (t_length < 4) {
      // we're in UTF-8 format
      from = utf_8;
    } else {
      // We need to figure out what kind of string we are
      // the choices are described in the table in the comments
      // above. Basically, we're going to do some "bit twiddling":
      int encoding = (0 != ttr[0] ? 8 : 0)
                   | (0 != ttr[1] ? 4 : 0)
                   | (0 != ttr[2] ? 2 : 0)
                   | (0 != ttr[3] ? 1 : 0);
      switch (encoding) {
      case 1  /*UTF-32BE*/: from = utf_32be; break;
      case 5  /*UTF-16BE*/: from = utf_16be; break;
      case 8  /*UTF-32LE*/: from = utf_32le; break;
      case 10 /*UTF-16LE*/: from = utf_16le; break;
      case 15 /*UTF-8*/:
      default:  from = utf_8; break; // why not?
      }
    }
    iconv_t cd = iconv_open("UTF-16LE", from);
    const std::size_t multiplier = 2;
    char obuffer[t_length*multiplier];
    char *obuf = obuffer;
    char *tbuf = const_cast<char*>(ttr);
    std::size_t in = t_length;
    std::size_t out = t_length*multiplier;
    std::size_t result = iconv(cd, &tbuf, &in, &obuf, &out);
    const std::size_t r_length = t_length*multiplier - out;
    iconv_close(cd);
    this->clear();
    this->resize(t_length*12); // worst-case scenario
    std::size_t offset = 0;
    for (std::size_t i=0; i<r_length; i+=2) {
      wchar_t value = (unsigned char)obuffer[i] + (unsigned char)(obuffer[i+1]<<8);
      if (31 < value and value < 127) {
        (*this)[offset] = obuffer[i];
        ++offset;
      } else {
        switch (value) {
        case '\t': (*this)[offset] = '\t'; ++offset; break;
        case '\v': (*this)[offset] = '\v'; ++offset; break;
        case '\n': (*this)[offset] = '\n'; ++offset; break;
        case '\r': (*this)[offset] = '\r'; ++offset; break;
        case '\b': (*this)[offset] = '\b'; ++offset; break;
        case '\f': (*this)[offset] = '\f'; ++offset; break;
        default: // large code-points
          (*this)[offset] = '\\';++offset;
          (*this)[offset] = 'u'; ++offset;
          (*this)[offset] = this->to_hex_value(value>>12); ++offset;
          (*this)[offset] = this->to_hex_value(value>> 8); ++offset;
          (*this)[offset] = this->to_hex_value(value>> 4); ++offset;
          (*this)[offset] = this->to_hex_value(value>> 0); ++offset;
          break;
        }
      }
    }
    this->resize(offset);
    std::cout << *this << std::endl;
  }
  inline char to_hex_value (std::size_t S) {
    return (15&S) + (((15&S) > 9) ? ('A'-10) : '0');
  }
  jstring (jstring const& jstr) : std::string(jstr) {}
  jstring& operator = (jstring const& jstr) {
    *static_cast<std::string*>(this) = *static_cast<const std::string*>(&jstr);
    return *this;
  }
};

//=== [ERROR MESSAGES] ===
struct unknown_identifier : std::exception {
  std::string message;
  template <typename String>
  unknown_identifier (String const& val) {
    this->message = "Not a valid identifier: "
      + to_ascii_string(val);
  }
  virtual ~unknown_identifier () throw() {}
  virtual const char* what () const throw() {
    return this->message.c_str();
  }
};

struct unknown_token : std::exception {
  std::string message;
  template <typename String>
  unknown_token (String const& val) {
    this->message = "Not a valid token: "
      + to_ascii_string(val);
  }
  virtual ~unknown_token () throw() {}
  virtual const char* what () const throw() {
    return this->message.c_str();
  }
};

struct unexpected_token : std::exception {
  std::string message;
  template <typename String>
  unexpected_token (String const& val) {
    this->message = "Unexpected token: "
      + to_ascii_string(val);
  }
  virtual ~unexpected_token () throw() {}
  virtual const char* what () const throw() {
    return this->message.c_str();
  }
};

struct expected_got : std::exception {
  std::string message;
  template <typename String>
  expected_got (String const& exp, String const& got) {
    this->message = "Expected a "
      + to_ascii_string(exp) + " got a "
      + to_ascii_string(got);
  }
  virtual ~expected_got () throw() {}
  virtual const char* what () const throw() {
    return this->message.c_str();
  }
};

//=== [traits class] ===
// The legal "values" for a JSON tree are listed below
// Also, used as a way to generate the "value_t" which
// is the top-level & recursive type. This construct
// allows us to use a boost::variant. OTOH, it may
// complicate other people's lives, because they'll
// have to know that value_t should be self-referential,
// in some cases. I.e. if they have a type T, then
// T::value_t is the same type as T, in some cases.
template <typename Type>
struct json_traits {
  typedef typename Type::value_t      value_t;
  typedef typename Type::string_t     string_t;
  typedef typename Type::number_t     number_t;
  typedef typename Type::object_t     object_t;
  typedef typename Type::array_t      array_t;
  typedef typename Type::bool_t       bool_t;
  typedef typename Type::null_t       null_t;
  // there are some semantic properties that also have to hold
  // 1. string_t should be semantically similar to std::basic_string
  // 2. number_t should be compatible with std::basic_stream
  // 3. object_t should have the following legal expressions:
  //     O[S] = V;
  //    where O is an object_t, S is a string_t, and V is one of the types above
  // 4. array_t should have the following legal expressions:
  //     A.push_back(V);
  //    where A is an array_t, and V is one of the types above
  // 5. bool_t should be assignable from bool
  // 6. null_t has no requirements, but should probably be cheap to move around!
};

//=== [parser generator] ===
// Given a type that satisfies the JSON type
template <typename JSONType>
struct push_parser {
  typedef json_traits<JSONType> traits;
  typedef typename traits::value_t      value_t;
  typedef typename traits::string_t     string_t;
  typedef typename traits::number_t     number_t;
  typedef typename traits::object_t     object_t;
  typedef typename traits::array_t      array_t;
  typedef typename traits::bool_t       bool_t;
  typedef typename traits::null_t       null_t;

  // JSON defines three identifiers:
  //   "true" "false" "null"
  // Because we don't know what type of string we'll be getting
  // we have to jump through some hoops; these values, below,
  // represent hoops.
  static const string_t True;
  static const string_t False;
  static const string_t Null;

  // so we know how to write stuff
  typedef typename string_t::value_type char_type;
  typedef std::basic_ostream<char_type> ostream;

  // short hand
  static string_t to_string_t (const char* cstr) {
    return string_t(cstr,cstr+std::strlen(cstr));
  }

private:
  // internal token class, should not be publically exposed
  // this thing is a little bulky because it stores a string
  // representation of everything it looks at. That means
  // that the tokens are about the same mass as the input string
  struct token {
    // The set of tokens; note that when we tokenize
    // we will convert identifiers into "boolean" or "null",
    // and character-strings that start/end with `"` become strings,
    // and character-strings that match the weird "number" spec
    // for JSON become "number".
    enum kind {
      unk = '?',
      curlyL = '{',
      curlyR = '}',
      brakL = '[',
      brakR = ']',
      string = '\"',
      number = 'n',
      colon = ':',
      comma = ',',
      boolean = 'b',
      null = '0',
    };
    token () : kind_(unk), value_(), offset_(0) {}

    // mainly for debug purposes
    friend ostream& operator << (ostream& ostr, token const& tok) {
      ostr << to_string_t("`") << to_ascii_string(tok.value_)
           << to_string_t("`@") << tok.offset_;
      return ostr;
    }
    kind kind_;           // which kind of token we are
    string_t value_;      // the string representation from the file
    std::size_t offset_;  // the offset into the file for printing purposes
                          // TODO: build an offset->(line,col) converter
  };

  typedef std::vector<token> tokens_t;
  typedef typename tokens_t::const_iterator tok_iter;
public:

  // given a string (filestr) whose contents are supposedly a JSON
  // try to parse; we return a recursive data-structure representing
  // the JSON file
  // the optional arguments are for printing the tokens, and where
  // to print them
  template <typename String>
  value_t operator () (String const& filestr) {
    return this->parse(filestr);
  }
  template <typename Iter>
  value_t operator () (Iter begin, Iter end) {
    return this->parse(begin,end);
  }

  // exactly like operator (), but with a name
  template <typename String>
  value_t parse (String const& filestr) {
    return parse(bel::begin(filestr),bel::end(filestr));
  }

  template <typename Iter>
  value_t parse (Iter begin, Iter end) {
      // make a copy of the input string into our internal
      // string type to simplify things, heavy-weight, but
      // we can optimize later
      string_t lcp(begin,end);
      // lex the file and get the tokens
      std::vector<token> tokens = this->lex(bel::begin(lcp),bel::end(lcp));
      // parse tokens
      return this->parse(tokens);
  }

private:
  // We're going to re-dispatch on the sequence [b,e) instead
  // of the token-container. This is a hand-written recursive
  // descent parser. Note that the JSON standard is "pseudo-regular"
  // so this is pretty easy to parse.
  value_t parse (tokens_t const& toks) {
    value_t val;
    tok_iter first = bel::begin(toks), last = bel::end(toks), prog;
    if (first != last) // prevent naughtiness
      prog = this->parse(first, last, val);
    return val;
  }

  // This function lexes a string into a list of tokens
  // it is NOT recursive, it is iterative.
  template <typename Iter>
  std::vector<token>
  lex (Iter first, Iter last) {
    std::vector<token> tokens;

    Iter begin = first, init = first;

    // Iterate over all the characters to generat tokens.
    // Since we're going to generate a token in each "pass"
    // through the while-loop, we create a default token (tok)
    // and set its initial values. The iterators, "first" and
    // "last" tell us where we're at in the list. The iterator
    // "init" tells us the global start-position (for calculating
    // the offset), and the iterator "begin" is used as a dummy
    // value.
    while (first != last) {
      token tok;
      tok.offset_ = first - init;
      tok.value_ = string_t(first,first+1);
      bool skip = false;
      // we're going to greedily eat the following things:
      // 1. strings "...", which include the legal escapes
      // 2. numbers 12.4e-35
      // 3. identifiers starting with "t" "f" or "n"
      // 4. comments, both c and c++ style
      // 5. the rest of the characters come in three groups:
      //    a. whitespace, which we ignore
      //    b. tokenizing chars: [ ] { } : ,
      //    c. error generating chars (everything else)
      switch (*first) {
      case ' ':case '\n':case '\v':case '\r':case '\b':case '\f':case '\t':
        skip = true; ++first; break;
      case '{': tok.kind_ = token::curlyL; ++first; break;
      case '}': tok.kind_ = token::curlyR; ++first; break;
      case '[': tok.kind_ = token::brakL; ++first; break;
      case ']': tok.kind_ = token::brakR; ++first; break;
      case ':': tok.kind_ = token::colon; ++first; break;
      case ',': tok.kind_ = token::comma; ++first; break;
      case '\"': {
          // strings start and end with a `"`
          // there are only a subset of legal escape sequences:
          //   1. whitespace \[bfnrt]
          //   2. unicode \u[0-9a-fA-F]*4, where '*4' means "four of them"
          //   3. other escape sequences \["\/]
          begin = first+1;
          // scan until we have an unescaped "
          tok.kind_ = token::string;
          while (first != last) {
            ++first;
            if ('\"' == *first) // end-of-string
              break;
            if ('\\' == *first) { // escape sequence
              ++first; // looking at the next character
              if (first == last) // ran out of characters
                throw unknown_token(to_string_t("\\"));
              switch (*first) {
              case '\"': case '\\': case '/':
              case 'b': case 'f': case 'n': case 'r': case 't':
                break; // fine, ignore these guys
              case 'u': { // unicode character-point format
                  // four hex digits, I assume this means: [0-9a-fA-F]
                  for (std::size_t i=0; i<4; ++i) {
                    ++first; // look at next char
                    if (first == last)
                      throw expected_got(
                        to_string_t("\\u[0-9a-fA-F]*4"),
                        string_t(begin,first));
                    if (not ((('0' <= *first) and (*first <= '9'))
                      or (('a' <= *first) and (*first <= 'f'))
                      or (('A' <= *first) and (*first <= 'F'))))
                      throw expected_got(
                        to_string_t("\\u[0-9a-fA-F]*4"),
                        string_t(first,first+1));
                  }
                } break;
              default: // uhoh
                throw unknown_token(
                        to_string_t("\\")+*first);
              }
            }
          }
          tok.value_ = string_t(begin, first);
          ++first; // eat last " character
        } break;
      case '0':case '1':case '2':case '3':case '4':
      case '5':case '6':case '7':case '8':case '9':
      case '-': {
          // numbers start with [0-9] or a '-'
          // they are of this format: -?[0-9]+(.[0-9]+)?([eE][+-][0-9]+)
          // which is a pretty ghetto integer/float format
          begin = first;
          // scan until we have a non-number char
          // the allowed characters are:
          tok.kind_ = token::number;
          // get digits portion
          if ('-' == *first) ++first; // get optional -
          first = get_digits(first,last); // get the digits
          // we could have a dot and some digits
          if ('.' == *first) {
            ++first; // if we have a ., we have to have more digits
            first = get_digits(first,last);
          }
          // optional "exponent" for our "mantissa"
          if ('e' == *first || 'E' == *first) {
            ++first;
            if ('-' == *first || '+' == *first) // optional sign
              ++first;
            first = get_digits(first,last);
          }
          tok.value_ = string_t(begin, first);
        } break;
      case 't': case 'f': case 'n': {
          // possibly an identifier, there are three legal ones:
          // "true", "false", and "null"
          // What we do is figure out which identifier it is by its first
          // character. Then we compare the identifier it *should* be
          // to the next k-characters. If it doesn't match (not the
          // right identifier, or not enough characters, whatever), we
          // abort.
          begin = first;
          tok.kind_ = token::boolean;
          string_t wh = True; // default to "true"
          if ('f' == *first) wh = False; // "false"
          else if ('n' == *first) {
            wh = Null; // "null"; also, change the token type
            tok.kind_ = token::null;
          }
          // compare the next few chars to our identifier
          typename string_t::const_iterator whs = bel::begin(wh), whd = bel::end(wh);
          while (first != last and whs != whd) {
            if (*first != *whs) {
              throw unknown_token(
                        string_t(begin,first));
            }
            ++first; ++whs;
          }
          tok.value_ = string_t(begin, first);
        } break;
      case '/': case '#': {
          // comments are actually an optional construt for JSON, but
          // they're so useful, it just feels right to have them; also,
          // this is more liberal than not having them, and they're pretty
          // easy to build
          // All comments start with "/"
          //   1. C++ continue (immediately) with "/" and go to "\n"
          //   2. C continue (immediately) with "*" and go to "*/"
          // consume first slash
          skip = true;
          const Iter orig = first;
          ++first;
          if (first == last) // / is not a legal anything
            throw unknown_token(
                    to_string_t("/"));
          if ('*' == *first) { // C-style comment
            // c-style
            while (first != last) {
              if ('*' == *first) { // look for a */
                ++first;
                if (first == last) // comment ended before */
                  throw unknown_token(
                          to_string_t("*"));
                if ('/' == *first) { // it is done!
                  ++first;
                  break;
                }
              }
              ++first;
            }
          } else if ('/' == *first) { // C++ style comment must have //
            // c++ style
            // go to the end of the line or file
            cppcomment: while (first != last) {
              if ('\n' == *first)
                break;
              ++first;
            }
            if (first != last)
              ++first;
          } else if ('#' == *orig)
            goto cppcomment; // blech
          else // /? is not legal
            throw unknown_token(string_t(first,first+1));
        } break;
      default: // don't know ... but also don't care (for now)
        tok.kind_ = token::unk;
        ++first;
      }
      if (not skip)
        tokens.push_back(tok);
    }
    return tokens;
  }

  template <typename StrIter>
  StrIter get_digits(StrIter first, StrIter last) {
    // scan, look for 0-9
    while (first != last) {
      if ('0' > *first || '9' < *first)
        break;
      ++first;
    }
    return first;
  }

  // The parser is recursive-descent...
  // This function tries to make a value, in some sense it is the
  // general case. We could have any of the different "types" in
  // a general value. There is a known 1-1 mapping from token to
  // type:
  //   " -> string_t
  //   n -> number_t
  //   { -> object_t
  //   [ -> array_t
  //   b -> true/false
  //   0 -> null
  // These are the only legal (top-level) tokens. We reject everything
  // else.
  tok_iter parse (tok_iter first, tok_iter last, value_t& val) {
    if (first == last) return last;
    string_t string;
    number_t number;
    object_t object;
    array_t  array;
    bool_t   boolean;
    null_t   null;
    switch (first->kind_) {
      case token::string:
        first = this->parse(first, last, string);
        val = string;
        break;
      case token::number:
        first = this->parse(first, last, number);
        val = number;
        break;
      case token::boolean:
        first = this->parse(first, last, boolean);
        val = boolean;
        break;
      case token::null:
        first = this->parse(first, last, null);
        val = null;
        break;
      case token::curlyL:
        first = this->parse(first, last, object);
        val = object;
        break;
      case token::brakL:
        first = this->parse(first, last, array);
        val = array;
        break;
      default:
        throw unexpected_token(first->value_);
        break;
    }
    return first;
  }

  // a string is a single token, just assign to the out value
  tok_iter parse (tok_iter first, tok_iter last, string_t& str) {
    if (first == last) return last;
    str = first->value_;
    return ++first;
  }
  // a number is a single token, just assign to the out value
  // NB: we build our own stringstream for conversion ... oy
  tok_iter parse (tok_iter first, tok_iter last, number_t& num) {
    if (first == last) return last;
    typedef std::basic_stringstream<typename string_t::value_type> sstream_t;
    sstream_t ss(first->value_);
    ss >> num; // this is where our requirement
               // for stringstream convertible comes from
    return ++first;
  }
  // a boolean is a single token, just assign the right value
  tok_iter parse (tok_iter first, tok_iter last, bool_t& b) {
    if (first == last) return last;
    if (True == first->value_)
      b = true;
    else if (False == first->value_)
      b = false;
    else // other kinds of identifier values are illegal
      throw unknown_identifier(first->value_);
    return ++first;
  }
  // a null is a single token, eat it
  tok_iter parse (tok_iter first, tok_iter last, null_t& n) {
    if (first == last) return last;
    if (Null != first->value_) // reject other kinds of identifiers
      throw unknown_identifier(first->value_);
    return ++first;
  }
  // an object is where we lose "regularity" for our language (along with
  // arrays):
  //    object ::= `{` (string : value [, string : value]*)? `}`
  // Where value could ALSO be an object.
  // However, there is still the "string : value" to maintain
  tok_iter parse (tok_iter first, tok_iter last, object_t& obj) {
    if (first == last) return last;
    // eat the {
    ++first;
    tok_iter prog; // we must guarantee progress
    string_t key;  // the key
    value_t val;   // the value
    do {
      // eat the key
      prog = this->parse(first, last, key);
      if (prog == first) // maintain progress
        break; // no key!
      first = prog;
      // eat the colon (:)
      if (token::colon != first->kind_)
        throw expected_got(to_string_t(":"),first->value_);
      ++first;
      // eat the value
      prog = this->parse(first, last, val);
      if (prog == first) // maintain progress
        throw expected_got(
                to_string_t("value"),
                to_string_t("nothing"));
      first = prog;
      // eat possible comma (,)
      obj[key] = val;
      // if there is a comma, there must be another "string : value"
      // pair; otherwise we're done
      if (token::comma != first->kind_)
        break;
      else
        ++first;
    } while (first != last);
    // had better be a }, if so, eat it
    if (first == last)
      throw expected_got(to_string_t("}"),
        to_string_t("nothing"));
    if (token::curlyR != first->kind_)
      throw expected_got(to_string_t("}"),first->value_);
    return ++first;
  }
  // arrays are a simpler versino of objects, the format is easier,
  // but also not regular:
  //    array ::= `[` (value [, value]*)? `]`
  // Note the nesting
  tok_iter parse (tok_iter first, tok_iter last, array_t& arr) {
    if (first == last) return last;
    // eat the [
    ++first;
    tok_iter prog;
    value_t val; // list of values
    do {
      prog = this->parse(first, last, val);
      if (prog == first) // maintain progress after looking for a value
        break;
      first = prog;
      // eat the comma (,)
      arr.push_back(val);
      // if we see a comma there had best be another value
      // (note it is because we eat this comma, then force progress on
      //  the loop that we get an error: there is a trailing `]' instead
      //  of a value!)
      if (token::comma != first->kind_)
        break;
      else
        ++first;
    } while (first != last);
    if (first == last)
      throw expected_got(to_string_t("]"),
        to_string_t("nothing"));
    if (token::brakR != first->kind_)
      throw expected_got(to_string_t("]"),first->value_);
    // eat the ]
    return ++first;
  }
};
// this kludginess allows us to easily look for identifiers
// welcome the wonderful world of Unicode!
static const char JSON__true[] = "true";
static const char JSON__false[] = "false";
static const char JSON__null[] = "null";
template <typename JsonType>
const typename json_traits<JsonType>::string_t
push_parser<JsonType>::True
  = typename json_traits<JsonType>::string_t(JSON__true, // yeah ... -1 ???
                                      JSON__true+sizeof(JSON__true)-1);
template <typename JsonType>
const typename json_traits<JsonType>::string_t
push_parser<JsonType>::False
  = typename json_traits<JsonType>::string_t(JSON__false,
                                      JSON__false+sizeof(JSON__false)-1);
template <typename JsonType>
const typename json_traits<JsonType>::string_t
push_parser<JsonType>::Null
  = typename json_traits<JsonType>::string_t(JSON__null,
                                      JSON__null+sizeof(JSON__null)-1);

//=== [PRINTER] ===
// the usual boost::variant visitor class
template <typename JsonType>
struct json_to_string {
  typedef JSONpp::json_traits<JsonType> json_type;
  typedef typename json_type::value_t   value_t;
  typedef typename json_type::string_t  string_t;
  typedef typename json_type::number_t  number_t;
  typedef typename json_type::object_t  object_t;
  typedef typename json_type::array_t   array_t;
  typedef typename json_type::bool_t    bool_t;
  typedef typename json_type::null_t    null_t;

  typedef typename string_t::value_type char_type;
  typedef std::basic_stringstream<char_type> bsstream;

  // short hand
  static string_t to_string_t (const char* cstr) {
    return string_t(cstr,cstr+std::strlen(cstr));
  }
  template <typename String>
  static string_t to_string_t (String const& str) {
    return string_t(bel::begin(str),bel::end(str));
  }

  json_to_string () {}

  struct __detail : boost::static_visitor<string_t> {
    string_t operator () (number_t const& N) const {
      bsstream bss;
      bss << N;
      return bss.str();
    }
    string_t operator () (string_t const& S) const {
      return to_string_t("\"") + S + to_string_t("\"");
    }
    string_t operator () (bool_t const& B) const {
      return to_string_t(B?"true":"false");
    }
    string_t operator () (null_t const& N) const {
      return to_string_t("null");;
    }
    string_t operator () (array_t const& A) const {
      typedef typename array_t::const_iterator citer;
      citer F = bel::begin(A), L = bel::end(A);
      string_t result = to_string_t("[");
      *this->index += 2;
      if (F != L) {
        if (this->pretty) {
          //result += string_t(1, '\n');
          result += string_t(1, ' ');
        }
        result += boost::apply_visitor(*this, *F);
        ++F;
      }
      while (F != L) {
        result += to_string_t(",");
        if (this->pretty) {
          result += string_t(1, '\n');
          result += string_t(*this->index, ' ');
        }
        result += boost::apply_visitor(*this, *F);
        ++F;
      }
      *this->index -= 2;
      if (this->pretty)
        result += string_t(1, ' ');
      result += to_string_t("]");
      return result;
    }
    string_t operator () (object_t const& O) const {
      typedef typename object_t::const_iterator citer;
      citer F = bel::begin(O), L = bel::end(O);
      string_t result = to_string_t("{");
      *this->index += 2;
      if (F != L) {
        if (this->pretty)
          result += string_t(1, ' ');
        result += to_string_t("\"")
          + F->first
          + to_string_t("\"");
        result += string_t(1, ':');
        if (this->pretty) {
          result += string_t(1, '\n');
          result += string_t(*this->index+2, ' ');
        }
        *this->index += 2;
        result += boost::apply_visitor(*this, F->second);
        *this->index -= 2;
        ++F;
      }
      while (F != L) {
        result += to_string_t(",");
        if (this->pretty) {
          result += string_t(1, '\n');
          result += string_t(*this->index, ' ');
        }
        result += to_string_t("\"") + F->first
               + to_string_t("\"");
        result += string_t(1, ':');
        if (this->pretty) {
          result += string_t(1, '\n');
          result += string_t(*this->index+2, ' ');
        }
        *this->index += 2;
        result += boost::apply_visitor(*this, F->second);
        *this->index -= 2;
        ++F;
      }
      *this->index -= 2;
      if (this->pretty)
        result += string_t(1, ' ');
      result += to_string_t("}");
      return result;
    }

    std::size_t*  index;
    bool          pretty;
  };

  string_t translate (value_t const& v, bool pp=false) const {
    std::size_t idx;
    __detail D;
    D.pretty = pp;
    D.index = &idx;
    return boost::apply_visitor(D, v);
  }
};

//=== [PREDEFINED JSON Type] ===
// a predefined family of JSON Types using boost::variant
// it is NOT compatible with pretty_print
struct nil {};

template <typename String=std::wstring,
  typename Double=double,
  typename Bool=bool,
  typename Null=nil>
struct make_json_value {

  typedef typename boost::make_recursive_variant<
    String,Double,Bool,Null,
    std::map<String,boost::recursive_variant_>,
    std::vector<boost::recursive_variant_> >::type type;

  typedef type                            value_t;
  typedef String                          string_t;
  typedef Double                          number_t;
  typedef std::map<String,type>           object_t;
  typedef std::vector<type>               array_t;
  typedef Bool                            bool_t;
  typedef Null                            null_t;
};

typedef make_json_value<> json_gen;
typedef json_gen::value_t json_v;

// specialization of the standard JSON type for
// the json_traits class
template <>
struct json_traits<json_v> {
  typedef json_v                        value_t;
  typedef std::wstring                  string_t;
  typedef double                        number_t;
  typedef std::map<std::wstring,json_v> object_t;
  typedef std::vector<json_v>           array_t;
  typedef bool                          bool_t;
  typedef nil                           null_t;
};

template <typename Iter>
json_v parse (Iter first, Iter last) {
  JSONpp::push_parser<json_v> parser;
  return parser(first, last);
}

json_gen::string_t
to_string (json_v const& value, bool pretty_print=false) {
  json_to_string<json_v> printer;
  return printer.translate(value, pretty_print);
}

//=== [JSON IOMANIPULATOR] ===
// this controls the format of the outputted JSON type
class iomanipulator_ {
  static signed long iword;
public:
  enum kinds {
    ascii       = 0,  // all unicode characters are \uXXXX | \UXXXXXXXX
    unicode     = 1,  // attempt to make unicode as unicode (not always possible)
    readable    = 2,  // spaces after commas & between structure control
    array_rc    = 4,  // new-line + indent for arrays
    array_first = 8,  // new-line for array_rc occurs after first element
    object_rc   = 16, // new-line + indent for objects
    object_key  = 32, // new-line + indent after key in an object
    ascii_all   = ascii | readable | array_rc | array_first | object_rc | object_key,
    unicode_all = unicode | readable | array_rc | array_first | object_rc | object_key,
    ascii_rc    = ascii | readable | array_rc | object_rc,
    unicode_rc  = unicode | readable | array_rc | object_rc,
  };
  iomanipulator_ (kinds const& k) : kind_(k) {
    if (-1 == iomanipulator_::iword)
      iomanipulator_::iword = std::ios_base::xalloc();
  }
  void set_format (std::ios_base& ios) const {
    ios.iword(iomanipulator_::iword) = this->kind_;
  }
  signed long format (std::ios_base& ios) const {
    return ios.iword(iomanipulator_::iword);
  }
private:
  kinds kind_;
};
signed long iomanipulator_::iword       = -1;
static const iomanipulator_ ascii       = iomanipulator_(iomanipulator_::ascii);
static const iomanipulator_ ascii_all   = iomanipulator_(iomanipulator_::ascii_all);
static const iomanipulator_ ascii_rc    = iomanipulator_(iomanipulator_::ascii_rc);
static const iomanipulator_ unicode     = iomanipulator_(iomanipulator_::unicode);
static const iomanipulator_ unicode_all = iomanipulator_(iomanipulator_::unicode_all);
static const iomanipulator_ unicode_rc  = iomanipulator_(iomanipulator_::unicode_rc);
static const iomanipulator_ readable    = iomanipulator_(iomanipulator_::readable);
static const iomanipulator_ array_rc    = iomanipulator_(iomanipulator_::array_rc);
static const iomanipulator_ array_first = iomanipulator_(iomanipulator_::array_first);
static const iomanipulator_ object_rc   = iomanipulator_(iomanipulator_::object_rc);
static const iomanipulator_ object_key  = iomanipulator_(iomanipulator_::object_key);

template <typename CharT>
std::basic_ostream<CharT>&
operator << (std::basic_ostream<CharT>& bostr, iomanipulator_ const& iom) {
  iom.set_format(bostr);
  return bostr;
}

}

#endif//JSON_PARSER
