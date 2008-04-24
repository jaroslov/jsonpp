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
  index = 'x',
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
  signed long value;

  token_t (kind_e k=unknown, signed long d=-1)
    : kind(k), value(d) {}

  friend std::ostream& operator << (std::ostream& ostr, token_t const& t) {
    ostr << "[" << (char)t.kind << "]=>"
      << t.value;
    return ostr;
  }
};

typedef std::vector<token_t> tokens_t;
typedef std::map<std::string,kind_e> name_kind_map_t;
typedef name_kind_map_t::const_iterator nkm_citer;
typedef std::vector<std::string> string_store_t;

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
  tokens_t
  operator () (Iter first, Iter last, string_store_t& string_store) const {
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
            // accessor:  [digits]
            ++first;
            if (first == last)
              throw std::runtime_error("Invalid predicate construction (no $ or digit)");
            if ('$' != *first)
              token.kind = index;
            else // skip passed the $
              ++first;
            if (first == last)
              throw std::runtime_error("Invalid predicate construction (no digits)");
            Iter start = first;
            while ((first != last) and std::isdigit(*first))
              ++first;
            if (first == last)
              throw std::runtime_error("Invalid predicate construction (not ])");
            std::stringstream ss(std::string(start,first));
            ss >> token.value;
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
            if (std::isalpha(*first)) {
              token.kind = identifier;
              Iter start = first;
              while ((first != last)
                and (std::isalnum(*first) or ('-' == *first)
                or ('_' == *first)))
                ++first;
              token.value = string_store.size();
              string_store.push_back(std::string(start,first));
              nkm_citer nkmtr = this->name_kind_map.find(
                                    string_store[token.value]);
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
    return tokens;
  }
};
static const lexer_t lexer = lexer_t();

struct code_t {
  kind_e axis;
  std::size_t test, predicate;

  friend std::ostream& operator << (std::ostream& ostr, code_t const& code) {
    ostr << (int)code.axis << " :: " << code.test << " [" << code.predicate << "]";
    return ostr;
  }
};
typedef std::vector<code_t> codes_t;

struct parser_t {
  template <typename Iter>
  codes_t operator () (Iter first, Iter last) {
    codes_t codes;
    // test
    // test pred
    // axis select test
    // axis select test pred
    while (first != last) {
      code_t code;
      bool use_code = true;
      switch (first->kind) {
      case axis: ++first; use_code = false; break;
      case identifier: {
          ++first; if (last == first) {
            // nothing else, we just have a test
            code.axis = first->kind;
            code.test = code.predicate = -1;
          } else switch (first->kind) {
          case select: {
              ++first; if (last == first)
                throw std::runtime_error(":: must be followed by a node-test");
              if (identifier != first->kind)
                throw std::runtime_error(":: must be followed by a node-test");
              code.test = first->value; // value holds the test string
              ++first; if (last == first) break;
              if (predicate != first->kind)
                break;
              // otherwise, fall through!
            }
          case predicate: case index: {
              code.predicate = first->value; // value holds the predicate
              ++first;
            } break;
          default:
            throw std::runtime_error("Malformed Axis"); break;
          }
        } break;
      default:
        throw std::runtime_error("Malformed Path"); break;
      }
      if (use_code) {
        std::cout << code << std::endl;
        codes.push_back(code);
      }
    }
    return codes;
  }
};
static parser_t parser = parser_t();

struct path {
  path (std::string const& P) {
    this->build_path(P);
  }
  void build_path (std::string const& P) {
    this->string_store.clear();
    // parse in place
    tokens_t tokens = lexer(bel::begin(P),bel::end(P),this->string_store);
    parser(bel::begin(tokens),bel::end(tokens));
  }
  string_store_t string_store;
};

}

#endif//VPATH_LIB
