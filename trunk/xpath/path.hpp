#include <bel/begin-end.hpp>
#include <stdexcept>
#include <cctype>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <cstring>

#include <iostream>
#include <iomanip>

#ifndef VPATH_LIB_PATH
#define VPATH_LIB_PATH

namespace vpath {

// the axis structure represents an axis from the xpath specification
// it contains three parts:
//   1. name -- one of the 13 actions (axes)
//   2. test -- the 'tag' value of the object to look for or some special
//       function, i.e., node(), text(), comment(), etc.
//   3. predicate -- some predicate to be applied to the result-set
struct axis_t {
  typedef signed long index_t;
  static const index_t None = -1;
  static const index_t WildCard = -2;
  static const index_t NilPredicate = -2;
  enum name_e {
    unknown = 'U',
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

  axis_t (name_e name=unknown, index_t test=None,
          index_t pred=None, bool fun=false)
    : name(name), test(test), predicate(pred), function(fun) {}

  // mainly for debuggin purposes
  friend std::ostream& operator << (std::ostream& ostr, axis_t const& a) {
    ostr << (char)a.name << "("
      << a.test << ","
      << a.predicate << ","
      << a.function << ")";
    return ostr;
  }

  name_e name;
  index_t test, predicate;
  bool function;
};
typedef std::vector<axis_t> axes_t;

// a custom manipulator for printing paths
class abbreviator {
  static signed long iword;
  enum kinds {
    abbreviate_E = 1,
    long_form_E = 4,
  };
public:
  abbreviator (bool abbr=true) : abbreviate(abbr) {
    if (-1 == abbreviator::iword)
      abbreviator::iword = std::ios_base::xalloc();
  }
  friend bool abbreviated (std::ios_base&);
  void format (std::ios_base& ios) const {
    ios.iword(abbreviator::iword)
      = this->abbreviate
          ? abbreviate_E
          : long_form_E;
  }
private:
  bool abbreviate;
};
long abbreviator::iword = -1;
bool abbreviated (std::ios_base& ios) {
  return ios.iword(abbreviator::iword) == abbreviator::abbreviate_E;
}
template <typename CharT>
std::basic_ostream<CharT>&
operator << (std::basic_ostream<CharT>& bostr, abbreviator const& abbr) {
  abbr.format(bostr);
  return bostr;
}
// let foo = "//one/two/@three
// std::cout << abbreviate << foo << std::endl;
// "//one/two/@three"
static abbreviator abbreviate = abbreviator(true);
// std::cout << long_form << foo << std::endl;
// "ancestor-or-self::one/child::two/attribute::three"
static abbreviator long_form = abbreviator(false);

// a path is our XPath representation of a path, it is (essentially)
// a list of axis_t, and a string-store; the string-store is where
// all the strings for the axes are kept
template <typename String=std::string>
struct path_type {
  typedef std::vector<String>           string_store_t;
  typedef typename String::value_type   char_type;
  typedef std::basic_ostream<char_type> bostream_t;

  path_type ()
    : axes(), string_store(), absolute(false) {}

  friend bostream_t& operator << (bostream_t& bostr, path_type const& P) {
    if (P.absolute)
      bostr << "/";
    for (std::size_t a=0; a<P.axes.size(); ++a) {
      bool refuse_function = false;
      switch (P.axes[a].name) {
      case axis_t::ancestor: bostr << "ancestor::"; break;
      case axis_t::ancestor_or_self:
        bostr << (abbreviated(bostr)?"/":"ancestor-or-self::"); break;
      case axis_t::attribute:
        bostr << (abbreviated(bostr)?"@":"attribute::"); break;
      case axis_t::child:
        bostr << (abbreviated(bostr)?"":"child::"); break;
      case axis_t::descendent: bostr << "descendent::"; break;
      case axis_t::descendent_or_self: bostr << "descendent-or-self::"; break;
      case axis_t::following: bostr << "following::"; break;
      case axis_t::following_sibling: bostr << "following-sibling::"; break;
      case axis_t::namespace_: bostr << "namespace::"; break;
      case axis_t::parent:
        refuse_function = abbreviated(bostr);
        bostr << (abbreviated(bostr)?"..":"parent::"); break;
      case axis_t::preceding: bostr << "preceding::"; break;
      case axis_t::preceding_sibling: bostr << "preceding-sibling::"; break;
      case axis_t::self:
        refuse_function = abbreviated(bostr);
        bostr << (abbreviated(bostr)?".":"self::"); break;
      default: bostr << "unknown"; break;
      }
      if (not refuse_function) {
        if (-1 < P.axes[a].test)
          bostr << P.string_store[P.axes[a].test];
        else if (axis_t::WildCard == P.axes[a].test)
          bostr << "*";
        if (P.axes[a].function)
          bostr << "()";
      }
      if (axis_t::None < P.axes[a].predicate)
        bostr << "[" << P.axes[a].predicate << "]";
      else if (axis_t::NilPredicate == P.axes[a].predicate)
        bostr << "[?]";
      if ((a+1) < P.axes.size())
        bostr << "/";
    }
    return bostr;
  }

  axis_t const& operator [] (std::size_t idx) const {
    return this->axes[idx];
  }
  std::size_t size () const { return this->axes.size(); }
  String test (std::size_t idx) const {
    return this->string_store[this->axes[idx].test];
  }

  axes_t          axes;
  string_store_t  string_store;
  bool            absolute;
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
      axis_root = 'R',
      axis_name = 'N',
      axis_test = 'T',
      axis_predicate = '$',
    };

    token_t (axis_t::name_e nm=axis_t::unknown,
      kind_e kd=axis_name, signed long idx=axis_t::None, bool fun=false)
      : name(nm), kind(kd), index(idx), function(fun) {}

    friend bostream_t& operator << (bostream_t& bostr, token_t const& token) {
      bostr << (char)token.kind << ""
            << (char)token.name << ":"
            << token.index
            << (token.function?"()":"");
      return bostr;
    }

    axis_t::name_e  name;
    kind_e          kind;
    signed long     index;
    bool            function;
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
    if (first != last) {
      tokens_t tokens = this->lex(first, last, path.string_store);
      path.absolute = this->parse(bel::begin(tokens), bel::end(tokens), path.axes);
    }
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
  bool parse (Iter first, Iter last, axes_t& axes) const {
    if (first == last)
      return false;
    axes.clear();
    bool absolute = false;
    if (token_t::axis_root == first->kind) {
      absolute = true;
      ++first;
    }
    while (first != last) {
      // axis-name? axis-test predicate?
      axis_t axis(axis_t::child); // default to a child
      if (token_t::axis_name == first->kind) {
        axis.name = first->name;
        ++first;
        if (first == last)
          throw std::runtime_error("(parser) an axis-test must follow an axis-name");
      }
      if (token_t::axis_test == first->kind) {
        axis.test = first->index;
        axis.function = first->function;
        ++first;
      } else
        throw std::runtime_error("(parser) expected an axis-test");
      if ((first != last) and (token_t::axis_predicate == first->kind)) {
        axis.predicate = first->index;
        ++first;
      }
      axes.push_back(axis);
    }
    return absolute;
  }
  template <typename Iter>
  Iter lex_identifier (Iter first, Iter last) const {
    if (first == last)
      return first;
    if (not (('-' == *first) or ('_' == *first)
      or ('&' == *first) or std::isalpha(*first)))
      return first;
    bool escape_sequence = false;
    while (first != last) {
      if ('&' == *first) {
        ++first;
        escape_sequence = true;
        continue;
      } else if (escape_sequence and (';' == *first)) {
        ++first;
        escape_sequence = true;
        continue;
      }
      if (('-' == *first) or ('_' == *first) or std::isalnum(*first)) {
        ++first;
        continue;
      }
      break;
    }
    return first;
  }
  template <typename Iter>
  tokens_t lex (Iter first, Iter last, strstore_t &strstore) const {
    tokens_t tokens;
    if (first == last)
      return tokens;
    if ('/' == *first) {
      tokens.push_back(token_t(axis_t::unknown,token_t::axis_root));
      ++first;
    }
    for (; first != last; ) {
      switch (*first) {
      case '[': {
          // eat and throw away the predicate, for now
          while ((first != last) and (']' != *first))
            ++first;
          if (first == last)
            throw std::runtime_error("(lexer) expected last `]`");
          tokens.push_back(token_t(axis_t::unknown,token_t::axis_predicate,
                                    axis_t::NilPredicate));
          ++first;
        } break;
      case '/': { // an descendent-or-self axis-name shorthand
          if ((first !=last) and ('/' == *(first+1))) {
            tokens.push_back(token_t(axis_t::descendent_or_self,token_t::axis_name));
            ++first;
          }
          ++first;
        } break;
      case '@': {
          ++first;
          tokens.push_back(token_t(axis_t::attribute,token_t::axis_name));
        } break;
      case '.': {
          if ((first != last) and ('.' == *(first+1))) {
            tokens.push_back(token_t(axis_t::parent,token_t::axis_name));
            ++first;
          } else {
            tokens.push_back(token_t(axis_t::self,token_t::axis_name));
          }
          // emulates a function-name: node ()
          tokens.push_back(token_t(axis_t::unknown,token_t::axis_test,
                                    strstore.size(),true));
          strstore.push_back("node");
          ++first;
        } break;
      case '*': { // wild card
          tokens.push_back(token_t(axis_t::unknown,token_t::axis_test,
                                    axis_t::WildCard));
          ++first;
        } break;
      default: {
          const Iter prog = this->lex_identifier(first, last);
          if (prog == first)
            throw std::runtime_error(
              (std::string("(lexer) unable to identify the next token: `")
                + char(*first) + "`").c_str());
          String id(first,prog);
          first = prog;
          skm_citer is_axis_name = this->str_kind_map.find(id);
          if ((this->str_kind_map.end() == is_axis_name)
            or (':' != *first)) { // if it is an axis-name, has to have a '::'
            tokens.push_back(token_t(axis_t::unknown,token_t::axis_test,
                                      strstore.size()));
            strstore.push_back(id);
            // if it is followed by a '()' then it is a function, as well
            if ('(' == *first) {
              ++first; if ((first == last) or (')' != *first))
                throw std::runtime_error("(lexer) missing closing `)` in `()`");
              ++first;
              tokens.back().function = true;
            }
          } else {
            if ((last == first) or (':' != *(first+1)))
              throw std::runtime_error("(lexer) missing second `:` in `::`");
            tokens.push_back(token_t(is_axis_name->second,token_t::axis_name));
            ++++first;
          }
        }
      }
    }
    return tokens;
  }
};

static const path_parser_generator<> parser = path_parser_generator<>();
typedef path_parser_generator<>::path_t path;

}// end namespace: vpath

#endif//VPATH_LIB_PATH
