#include <begin-end.hpp>
#include <stdexcept>
#include <cctype>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <cstring>

#include <iostream>

#ifndef VPATH_LIB_PATH
#define VPATH_LIB_PATH

namespace vpath { namespace path {

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
struct path_type {
  typedef std::vector<String>           string_store_t;
  typedef typename String::value_type   char_type;
  typedef std::basic_ostream<char_type> bostream_t;

  path_type () {}

  friend bostream_t& operator << (bostream_t& bostr, path_type const& P) {
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
        bostr << "[" << P.axes[a].predicate << "]";
    }
    return bostr;
  }

  axes_t          axes;
  string_store_t  string_store;
};

template <typename String=std::string>
struct path_parser_generator {
  typedef path_type<String>                   path_t;
  typedef typename String::value_type         char_type;
  typedef std::basic_stringstream<char_type>  bsstream_t;
  typedef std::basic_ostream<char_type>       bostream_t;
private:
  typedef typename path_t::string_store_t strstore_t;
  struct token_t {
    enum kind_e {
      axis_name = 'N',
      axis_test = 'T',
      axis_predicate = '$',
    };

    token_t (axis_t::name_e nm=axis_t::unknown,
      kind_e kd=axis_name, std::size_t idx=0)
      : name(nm), kind(axis_name), index(0) {}

    friend bostream_t& operator << (bostream_t& bostr, token_t const& token) {
      bostr << (char)token.kind
            << (char)token.name
            << token.index;
      return bostr;
    }

    axis_t::name_e  name;
    kind_e          kind;
    std::size_t     index;
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
    //this->parse(bel::begin(tokens), bel::end(tokens), path.axes);
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
  template <typename Iter>
  tokens_t lex (Iter first, Iter last, strstore_t &strstore) const {
    tokens_t tokens;
    for (; first != last; ++first) {
      switch (*first) {
      case '/': {
          if ((first !=last) && ('/' == *(first+1))) {
            tokens.push_back(token_t(axis_t::ancestor_or_self,token_t::axis_test));
          }
        } break;
      }
    }
    return tokens;
  }
};

static const path_parser_generator<> parser = path_parser_generator<>();
typedef path_parser_generator<>::path_t path;

  }// namespace: path
}// namespace: vpath

#endif//VPATH_LIB_PATH
