#include <begin-end.hpp>
#include <stdexcept>
#include <cctype>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <cstring>

#include <iostream>

#ifndef VPATH_LIB
#define VPATH_LIB

namespace vpath {

struct axis_t {
  typedef signed long index_t;
  static const index_t None = -1;
  static const index_t WildCard = -2;
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
      bostr << "/";
      switch (P.axes[a].name) {
      case axis_t::ancestor: bostr << "ancestor"; break;
      case axis_t::ancestor_or_self: bostr << "ancestor-or-self"; break;
      case axis_t::attribute: bostr << "attribute"; break;
      case axis_t::child: bostr << "child"; break;
      case axis_t::descendent: bostr << "descendent"; break;
      case axis_t::descendent_or_self: bostr << "descendent-or-self"; break;
      case axis_t::following: bostr << "following"; break;
      case axis_t::following_sibling: bostr << "following-sibling"; break;
      case axis_t::namespace_: bostr << "namespace"; break;
      case axis_t::parent: bostr << "parent"; break;
      case axis_t::preceding: bostr << "preceding"; break;
      case axis_t::preceding_sibling: bostr << "preceding-sibling"; break;
      case axis_t::self: bostr << "self"; break;
      default: bostr << "unknown"; break;
      }
      if (-1 < P.axes[a].test)
        bostr << "::" << P.string_store[P.axes[a].test];
      else if (axis_t::WildCard == P.axes[a].test)
        bostr << "::*";
      if (-1 != P.axes[a].predicate)
        bostr << "[$" << P.axes[a].predicate << "]";
    }
    return bostr;
  }

  axes_t          axes;
  string_store_t  string_store;
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
      bostr << (char)token.kind_
            << (char)token.name_
            << token.index_;
      return bostr;
    }

    axis_t::name_e  name_;
    token_kind_e    kind_;
    std::size_t     index_;
  };
  typedef std::vector<token_t>                    tokens_t;
  typedef typename tokens_t::const_iterator       tok_citer;
  typedef std::map<String,axis_t::name_e>         str_kind_map_t;
  typedef typename str_kind_map_t::const_iterator skm_citer;

  // members (constant)
  str_kind_map_t str_kind_map;
public:
  path_parser_generator () {
    this->str_kind_map[to_str("ancestor")] = axis_t::ancestor;
    this->str_kind_map[to_str("ancestor-or-self")] = axis_t::ancestor_or_self;
    this->str_kind_map[to_str("attribute")] = axis_t::attribute;
    this->str_kind_map[to_str("child")] = axis_t::child;
    this->str_kind_map[to_str("descendent")] = axis_t::descendent;
    this->str_kind_map[to_str("descendent-or-self")] = axis_t::descendent_or_self;
    this->str_kind_map[to_str("following")] = axis_t::following;
    this->str_kind_map[to_str("following-sibling")] = axis_t::following_sibling;
    this->str_kind_map[to_str("namespace")] = axis_t::namespace_;
    this->str_kind_map[to_str("parent")] = axis_t::parent;
    this->str_kind_map[to_str("preceding")] = axis_t::preceding;
    this->str_kind_map[to_str("preceding-sibling")] = axis_t::preceding_sibling;
    this->str_kind_map[to_str("self")] = axis_t::self;
  }
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
      tok_citer prog = first;
      switch (first->kind_) {
      case token_t::separator: ++first; break; // skip separators
      case token_t::identifier: {
          axis_t axis;
          axis.name = axis_t::child;
          axis.test = first->index_;
          // we know from the lexer that "unknown" identifiers
          // must be "child", so we keep the default above
          // otherwise we have an axis-name axis-test
          if (axis_t::unknown != first->name_) {
            axis.name = first->name_;
            if (first == last)
              throw std::runtime_error("The axis-name does not have an axis-test!");
            ++first;
            if ((axis_t::unknown == first->name_)
              and (token_t::identifier == first->kind_))
              axis.test = first->index_;
            else
              throw std::runtime_error("The axis-test must be an identifier!");
            ++first;
          } else
            ++first; // eat the bare identifier
          // get optional predicate
          if (first != last) {
            if (token_t::index == first->kind_) {
              axis.predicate = first->index_;
              throw std::runtime_error("Failed to build the index predicate!");
              ++first;
            } else if (token_t::predicate == first->kind_) {
              axis.predicate = first->index_;
              ++first;
            }
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
            axis.test = first->index_;
          else
            axis.predicate = first->index_;
          axes.push_back(axis);
          ++first;
        } break;
      default:
        throw std::runtime_error("Unexpected token!");
      }
      if (prog == first)
        throw std::runtime_error("Internal error: no progress (parser)");
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
      case '*': { // wild-card child identifier
          ++first;
          token.kind_ = token_t::identifier;
          token.index_ = axis_t::WildCard;
        } break;
      default: { // accepts identifiers
          token.kind_ = token_t::identifier;
          String id;
          const Iter prog = this->lex_identifier(first, last, id);
          if (prog == first)
            throw std::runtime_error("There was no legal identifier!");
          first = prog;
          skm_citer skmtr = this->str_kind_map.find(id);
          if (this->str_kind_map.end() != skmtr)
            token.name_ = skmtr->second;
          else
            token.name_ = axis_t::unknown;
          bool is_test_name = true;
          if (axis_t::unknown != token.name_) {
            // it is only a *real* axis-name if it is followed by `::`
            if (first != last) {
              if (':' == *(first+1)) { // there is a selector!
                ++first; if ((last == first) or (':' != *first))
                  throw std::runtime_error("The selector (::) is missing the last `:`");
                ++first; // eat the second `:`
                is_test_name = false; // it is NOT an axis-test!
              }
            }
          }
          if (is_test_name) {
            token.name_  = axis_t::unknown;
            token.index_ = store.size();
            store.push_back(id);
          }
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
