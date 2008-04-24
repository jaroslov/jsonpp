#include <begin-end.hpp>
#include <stdexcept>
#include <cctype>
#include <vector>
#include <string>
#include <sstream>

#include <iostream>

#ifndef VPATH_LIB
#define VPATH_LIB

namespace vpath {

struct axis_t {
  typedef signed long index_t;
  static const index_t None = -1;
  enum name_e {
    unknown = '0',
    ancestor = 'a',
    ancestor_or_self = 'A',
    attribute = '@',
    child = 'c',
    descendent = 'd',
    descendent_or_self = 'D',
    following = 'f',
    following_sibling = 'F',
    namespace_ = 'n',
    parent = '^',
    preceding = 'p',
    preceding_sibling = 'P',
    self = 's',
  };

  axis_t (name_e name=unknown, index_t test=None, index_t pred=None)
    : name(name), test(test), predicate(pred) {}

  name_e name;
  index_t test, predicate;
};
typedef std::vector<axis_t> axes_t;

template <typename String=std::string>
struct path {
  typedef std::vector<String> string_store_t;

  path () {}

  axes_t axes;
  string_store_t string_store;
};

template <typename String=std::string>
struct path_parser_generator {
  typedef path<String>                    path_t;
private:
  typedef typename path_t::string_store_t strstore_t;
  struct token_t {
    enum token_kind_e {
      select = ':',
      predicate = '$',
      index = '#',
      separator = '/',
      ancestor_or_self = 'A',
      self = '.',
      parent = '^',
      attribute_predicate = '&',
      attribute_named = '@',
      identifier = 'i',
    };

    token_t () : kind_(self), index_(0) {}

    token_kind_e kind_;
    std::size_t  index_;
  };
  typedef std::vector<token_t> tokens_t;
public:
  path_t operator () (String const& pathstr) const {
    return (*this)(bel::begin(pathstr), bel::end(pathstr));
  }
  template <typename Iter>
  path_t operator () (Iter first, Iter last) const {
    path_t path;
    tokens_t tokens = this->lex(first, last, path.string_store);
    return path;
  }
private:
  template <typename Iter>
  Iter lex_digits (Iter first, Iter last, std::size_t& val) const {
    typedef typename String::value_type char_type;
    typedef std::basic_stringstream<char_type> bsstream_t;
    Iter start = first;
    while ((first != last) and std::isdigit(*first))
      ++first;
    String str(start,first);
    bsstream_t bss(str);
    bss >> val;
    return first;
  }
  template <typename Iter>
  Iter lex_identifier (Iter first, Iter last, String& str) const {
    if (not (std::isalpha(*first) or ('_' == *first) or ('-' == *first)))
      return first;
    Iter start = first;
    while ((first != last)
      and (std::isalnum(*first) or ('_' == *first) or ('-' == *first)))
      ++first;
    str = String(start,first);
    return first;
  }
  template <typename Iter>
  tokens_t lex (Iter first, Iter last, strstore_t& store) const {
    // lexemes:
    //   ::                select
    //   [$digits]         predicate
    //   [digits]          index
    //   /                 separator
    //   //                ancestor_or_self
    //   .                 self
    //   ..                parent
    //   @$digits          attribute_predicate
    //   @identifier       attribute_named
    //   identifier        identifier
    //
    //   what is identifier?
    //   [a-zA-Z][a-zA-Z0-9\-\_]*
    tokens_t tokens;
    while (first != last) {
      std::cout << *first << std::endl;
      token_t token;
      switch (*first) {
      case ':' : { // ::
          ++first; if ((first == last) or (':' != *first))
            throw std::runtime_error("Final \':\' is missing.");
          ++first;
          token.kind_ = token_t::separator;
        } break;
      case '/' : { // / | //
          token.kind_ = token_t::separator;
          ++first; if ((first != last) and ('/' == *first)) {
            token.kind_ = token_t::ancestor_or_self;
            ++first;
          }
        } break;
      case '.' : { // . | ..
          token.kind_ = token_t::self;
          ++first; if ((first != last) and ('.' == *first)) {
            token.kind_ = token_t::parent;
            ++first;
          }
        } break;
      case '[' : { // [$digits] | [digits]
          token.kind_ = token_t::index; // default to index accessor
          ++first; if (first == last)
            throw std::runtime_error("Malformed predicate or index-accessor.");
          if ('$' == *first) {
            token.kind_ = token_t::predicate; // a predicate
            ++first;
          }
          const Iter prog = this->lex_digits(first, last, token.index_);
          if (prog == first)
            throw std::runtime_error("There was no numeric value in predicate or index!");
          first = prog;
          if (']' != *first)
            throw std::runtime_error("Malformed predicate or index-accessor. (no \']\')");
          ++first; // eat last ]
        } break; ']'; // see is dumb sometimes
      case '@' : { // @$digits | @identifier
          ++first; if (first == last)
            throw std::runtime_error("Malformed attribute, no test!");
          if ('$' == *first) {
            ++first; // eat $
          } else {
          }
        } break;
      default: { // accepts identifiers
          token.kind_ = token_t::identifier;
          String id;
          const Iter prog = this->lex_identifier(first, last, id);
          if (prog == first)
            throw std::runtime_error("There was no legal identifier!");
          store.push_back(id);
          first = prog;
        }
      }
      tokens.push_back(token);
    }
  }
};

static const path_parser_generator<> strparser = path_parser_generator<>();

}

#endif//VPATH_LIB
