#include <stdexcept>
#include <cctype>

#ifndef VPATH_LIB
#define VPATH_LIB

namespace vpath {

enum kind_e {
  unknown = '0',
  ancestor = 'a',
  ancestor_or_self = 'A',
  attribute = '@',
  axis = '/',
  child = 'c',
  descendent = ',',
  descendent_or_self = ';',
  following = 'f',
  following_sibling = 'F',
  identifier = 'I',
  namespace_ = 'n',
  parent = ':',
  preceding = 'p',
  preceding_sibling = 'P',
  predicate = '$',
  select = 's',
  self = '.',
};

struct token_t {
  kind_e kind;
  std::string identifier;
  std::size_t digit;

  token_t (kind_e k=unknown)
    : kind(k), identifier(""), digit(0) {}
  token_t (kind_e k, std::string const& i)
    : kind(k), identifier(i), digit(0) {}
  token_t (kind_e k, std::size_t const& d)
    : kind(k), identifier(""), digit(d) {}

  friend std::ostream& operator << (std::ostream& ostr, token_t const& t) {
    ostr << (char)t.kind << "] \""
      << t.identifier << "\" ("
      << t.digit  << ")";
    return ostr;
  }
};

typedef std::vector<token_t> tokens_t;
typedef std::map<std::string,kind_e> name_kind_map_t;
typedef name_kind_map_t::const_iterator nkm_citer;

struct lexer_t {
  name_kind_map_t name_kind_map;

  lexer_t () {
    this->name_kind_map["ancestor"] = ancestor;
    this->name_kind_map["ancestor-or-self"] = ancestor_or_self;
    this->name_kind_map["attribute"] = attribute;
    this->name_kind_map["child"] = child;
    this->name_kind_map["descendent"] = descendent;
    this->name_kind_map["descendent-or-self"] = descendent_or_self;
    this->name_kind_map["following"] = following;
    this->name_kind_map["following-sibling"] = following_sibling;
    this->name_kind_map["namespace"] = namespace_;
    this->name_kind_map["parent"] = parent;
    this->name_kind_map["preceding"] = preceding;
    this->name_kind_map["preceding-sibling"] = preceding_sibling;
    this->name_kind_map["self"] = self;
  }

  template <typename Iter>
  void operator () (Iter first, Iter last) const {
    tokens_t tokens;
    while (first != last) {
      token_t token;
      switch (*first) {
      case '/' : {
          // if it is "//" then it is a descendent-or-self
          // else we have "/" then it is an axis
          if (((first+1) != last) and ('/' == *(first+1))) {
            token.kind = descendent_or_self;
            ++first;
          } else
            token.kind = axis;
          ++first;
        } break;
      case '[' : {
          token.kind = predicate;
          // predicate: [$digits]
          ++first;
          if (first == last)
            throw std::runtime_error("Invalid predicate construction (no $)");
          if ('$' != *first)
            throw std::runtime_error("Invalid predicate construction (not $)");
          ++first;
          if (first == last)
            throw std::runtime_error("Invalid predicate construction (no digits)");
          Iter start = first;
          while ((first != last) and std::isdigit(*first))
            ++first;
          if (first == last)
            throw std::runtime_error("Invalid predicate construction (not ])");
          std::stringstream ss(std::string(start,first));
          ss >> token.digit;
          if (']' != *first)
            throw std::runtime_error("Invalid predicate construction (not ])");
          ++first;
        } break;
        case '.' : {
            // if it is "//" then it is a descendent-or-self
            // else we have "/" then it is an axis
            if (((first+1) != last) and ('.' == *(first+1))) {
              token.kind = parent;
              ++first;
            } else
              token.kind = self;
            ++first;
          } break;
        case ':' : {
            ++first;
            if (first == last)
              throw std::runtime_error("Invalid predicate construction (no :)");
            if (':' != *first)
              throw std::runtime_error("Invalid predicate construction (not :)");
            ++first;
            token.kind = select;
          } break;
        case '`' : {
            Iter start = first;
            ++first;
            while ((first != last) and ('\'' != *first))
              ++first;
            if (first == last)
              throw std::runtime_error("Invalid literal (no \')");
            if ('\'' != *first)
              throw std::runtime_error("Invalid literal (not \')");
            ++first;
            token.kind = identifier;
            token.identifier = std::string(start+1,first-1);
          } break;
        default: {
            if (std::isalpha(*first)) {
              token.kind = identifier;
              Iter start = first;
              while ((first != last) and std::isalnum(*first))
                ++first;
              token.identifier = std::string(start,first);
              nkm_citer nkmtr = this->name_kind_map.find(token.identifier);
              if (this->name_kind_map.end() != nkmtr)
                token.kind = nkmtr->second;
            } else
              throw std::runtime_error("Unknown character");
          } break;
      }
      tokens.push_back(token);
      if (first == last)
        break;
    }
  }
};
static const lexer_t lexer = lexer_t();

struct path {
  path (std::string const& P) {
    this->build_path(P);
  }
  void build_path (std::string const& P) {
    // parse in place
    typedef std::string::const_iterator str_iter;
    lexer(bel::begin(P),bel::end(P));
  }
};

}

#endif//VPATH_LIB
