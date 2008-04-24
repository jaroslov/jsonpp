#include <stdexcept>
#include <cctype>

#ifndef VPATH_LIB
#define VPATH_LIB

namespace vpath {

struct path {
  enum kind_e {
    unknown = '0',
    axis = 'a',
    self = '.',
    child = '/',
    parent = ':',
    predicate = '$',
    descendent_or_self = ';',
    select = 's',
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

  path (std::string const& P) {
    this->build_path(P);
  }
  void build_path (std::string const& P) {
    // parse in place
    typedef std::string::const_iterator str_iter;
    this->lex(bel::begin(P),bel::end(P));
  }
  template <typename Iter>
  void lex (Iter first, Iter last) const {
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
        default: {
            
            ++first;
          } break;
      }
      std::cout << token << std::endl;
      tokens.push_back(token);
      if (first == last)
        break;
    }
  }
};

}

#endif//VPATH_LIB
