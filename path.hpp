#include <begin-end.hpp>
#include <stdexcept>
#include <cctype>
#include <vector>
#include <string>

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

  path_t operator () (String const& pathstr) const {
    return (*this)(bel::begin(pathstr), bel::end(pathstr));
  }
  template <typename Iter>
  path_t operator () (Iter first, Iter last) const {
    path_t path;
    this->lex(first, last, path.string_store);
    return path;
  }
private:
  typedef typename path_t::string_store_t strstore_t;

  template <typename Iter>
  void lex (Iter first, Iter last, strstore_t& store) const {
    // lexemes:
    //   ::
    //   [$digits]
    //   [digits]
    //   /
    //   //
    //   .
    //   ..
    //   @$digits
    //   @identifier
    //   identifier
    //
    //   what is identifier?
    //   [a-zA-Z][a-zA-Z0-9\-\_]*
    while (first != last) {
    case ':': { // ::
      } break;
    case '[': { // [$digits] | [digits]
      } break;
    case '/': { // / | //
      } break;
    case '.': { // . | ..
      } break;
    case '@': { // @$digits | @identifier
      } break;
    default: { // identifier
      }
    }
  }
};

static const path_parser_generator<> strparser = path_parser_generator<>();

}

#endif//VPATH_LIB
