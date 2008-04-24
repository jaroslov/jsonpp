#include <begin-end.hpp>
#include <stdexcept>
#include <cctype>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>

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

  friend std::ostream& operator << (std::ostream& ostr, axis_t const& a) {
    ostr << (char)a.name << "(" << a.test << "," << a.predicate << ")";
    return ostr;
  }

  name_e name;
  index_t test, predicate;
};
typedef std::vector<axis_t> axes_t;

template <typename String=std::string>
struct path {
  typedef std::vector<String>           string_store_t;
  typedef typename String::value_type   char_type;
  typedef std::basic_ostream<char_type> bostream_t;

  path () {}

  friend bostream_t& operator << (bostream_t& bostr, path const& P) {
    for (std::size_t a=0; a<P.axes.size(); ++a) {
      bostr << "/" << (char)P.axes[a].name;
      if (-1 != P.axes[a].test)
        bostr << "::" << P.string_store[P.axes[a].test];
      if (-1 != P.axes[a].predicate)
        bostr << "[$" << P.axes[a].predicate << "]";
    }
    return bostr;
  }

  axes_t axes;
  string_store_t string_store;
};

template <typename String=std::string>
struct path_parser_generator {
  typedef path<String>                        path_t;
  typedef typename String::value_type         char_type;
  typedef std::basic_stringstream<char_type>  bsstream_t;
  typedef std::basic_ostream<char_type>       bostream_t;
private:
  typedef typename path_t::string_store_t strstore_t;
  struct token_t {
    enum token_kind_e {
      selector = ':',
      predicate = '$',
      index = '#',
      separator = '/',
      indexed_attribute = '&',
      named_attribute = '@',
      identifier = 'i',
    };

    token_t () : name_(axis_t::unknown), kind_(identifier), index_(0) {}

    friend bostream_t& operator << (bostream_t& bostr, token_t const& token) {
      bostr << "[" << (char)token.kind_ << "]{"
            << (int)token.kind_ << "w/" << (char)token.name_ << "}("
            << token.index_ << ")";
      return bostr;
    }

    axis_t::name_e  name_;
    token_kind_e    kind_;
    std::size_t     index_;
  };
  typedef std::vector<token_t>                tokens_t;
  typedef typename tokens_t::const_iterator   tok_citer;
public:
  path_t operator () (String const& pathstr) const {
    return (*this)(bel::begin(pathstr), bel::end(pathstr));
  }
  template <typename Iter>
  path_t operator () (Iter first, Iter last) const {
    path_t path;
    tokens_t tokens = this->lex(first, last, path.string_store);
    this->parse(bel::begin(tokens), bel::end(tokens), path.axes);
    return path;
  }
private:
  template <typename Str>
  static String to_str (Str const& str) {
    return String(str.begin(),str.end());
  }
  template <typename Char>
  static String to_str (const Char* str) {
    return String(str,std::strlen(str)+str);
  }
  void parse (tok_citer first, tok_citer last, axes_t& axes) const {
    axes.clear();
    while (first != last) {
      std::cout << *first << std::endl;
      tok_citer prog = first;
      switch (first->kind_) {
      case token_t::separator: ++first; break; // skip separators
      case token_t::identifier: {
          axis_t axis;
          if ((first == last)
            or (axis_t::unknown == first->name_)
            or (token_t::separator == (first+1)->kind_)) {
            axis.name = axis_t::child;
            axis.test = first->index_;
            ++first;
            if (first != last)
              ++first;
          } else {
            /*tok_citer next = first+1;
            if ((token_t::selector == next->kind_)
              and (axis_t::unknown != first->name_)) {
              axis.name = first->name_;
              ++++first; if (first == last)
                throw std::runtime_error("Malformed axis: no test!");
              axis.test = first->index_;
              ++first; // eat the test
            } else
              throw std::runtime_error("Unknown axis name!");
            if ((first != last) {
              if (token_t::index == first->kind_) {
                axis.predicate = first->index_;
                throw std::runtime_error("Failed to build the index predicate!");
                ++first;
              } else if (token_t::predicate == first->kind_) {
                axis.predicate = first->index_;
                ++first;
              }
            }*/
          }
          axes.push_back(axis);
        } break;
      case token_t::named_attribute:
      case token_t::indexed_attribute: {
          // named/indexed_attribute
          // named/indexed_attribute predicate
          axis_t axis;
          axis.name = axis_t::attribute;
          if (token_t::named_attribute == first->kind_)
            axis.predicate = first->index_;
          else
            axis.test = first->index_;
          axes.push_back(axis);
          ++first;
        } break;
      default: {
          std::cout << "DEFAULT " << *first << std::endl;
        }
      }
      if (prog == first)
        //throw std::runtime_error("Internal error: no progress (parser)");
        return;
    }
  }
  template <typename Iter>
  Iter lex_digits (Iter first, Iter last, std::size_t& val) const {
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
    //   @$digits          indexed_attribute
    //   @identifier       named_attribute
    //   identifier        identifier
    //
    //   what is identifier?
    //   [a-zA-Z][a-zA-Z0-9\-\_]*
    tokens_t tokens;
    while (first != last) {
      const Iter prog = first;
      token_t token;
      switch (*first) {
      case ':' : { // ::
          ++first; if ((first == last) or (':' != *first))
            throw std::runtime_error("Final \':\' is missing.");
          ++first;
          token.kind_ = token_t::selector;
        } break;
      case '/' : { // / | //
          token.kind_ = token_t::separator;
          ++first; if ((first != last) and ('/' == *first)) {
            token.kind_ = token_t::identifier;
            token.name_ = axis_t::ancestor_or_self;
            ++first;
          }
        } break;
      case '.' : { // . | ..
          token.kind_ = token_t::identifier;
          token.name_ = axis_t::self;
          ++first; if ((first != last) and ('.' == *first)) {
            token.name_ = axis_t::parent;
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
            token.kind_ = token_t::indexed_attribute;
            ++first; // eat $
            if (first == last)
              throw std::runtime_error("Malformed attribute: no index!");
            const Iter prog = this->lex_digits(first, last, token.index_);
            if (prog == first)
              throw std::runtime_error("Malformed attribute: no index!");
            first = prog;
          } else {
            throw std::runtime_error("Predicate expressions are unsupported.");
            token.kind_ = token_t::named_attribute;
            String id;
            const Iter prog = this->lex_identifier(first, last, id);
            if (prog == first)
              throw std::runtime_error("Malformed attribute: no test!");
            first = prog;
            token.index_ = store.size();
            store.push_back(id);            
          }
        } break;
      default: { // accepts identifiers
          token.kind_ = token_t::identifier;
          String id;
          const Iter prog = this->lex_identifier(first, last, id);
          if (prog == first)
            throw std::runtime_error("There was no legal identifier!");
          if (to_str("ancestor") == id)
            token.name_ = axis_t::ancestor;
          else if (to_str("ancestor-or-self") == id)
            token.name_ = axis_t::ancestor_or_self;
          else if (to_str("attribute") == id)
            token.name_ = axis_t::attribute;
          else if (to_str("child") == id)
            token.name_ = axis_t::child;
          else if (to_str("descendent") == id)
            token.name_ = axis_t::descendent;
          else if (to_str("descendent-or-self") == id)
            token.name_ = axis_t::descendent_or_self;
          else if (to_str("following") == id)
            token.name_ = axis_t::following;
          else if (to_str("following-sibling") == id)
            token.name_ = axis_t::following_sibling;
          else if (to_str("namespace") == id)
            token.name_ = axis_t::namespace_;
          else if (to_str("parent") == id)
            token.name_ = axis_t::parent;
          else if (to_str("preceding") == id)
            token.name_ = axis_t::preceding;
          else if (to_str("preceding-sibling") == id)
            token.name_ = axis_t::preceding_sibling;
          else if (to_str("self") == id)
            token.name_ = axis_t::self;
          else
            token.name_ = axis_t::unknown;
          token.index_ = store.size();
          store.push_back(id);
          first = prog;
        }
      }
      if (prog == first)
        throw std::runtime_error("Internal error: no progress (lexer)");
      tokens.push_back(token);
    }
    return tokens;
  }
};

static const path_parser_generator<> strparser = path_parser_generator<>();
typedef path_parser_generator<>::path_t path_type;

}

#endif//VPATH_LIB
