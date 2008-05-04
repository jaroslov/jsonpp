//#include <xpgtl/predicate.hpp>
#include <string>
#include <stdexcept>

template <typename String>
struct token_t 
  typedef String string_t;
  typedef token_t<string_t> self_type;

  token_t () : value() {}

  friend bool operator == (self_type const& L, self_type const& R)
    { return L.value == R.value; }
  friend bool operator == (string_t const& L, self_type const& R)
    { return L == R.value; }
  friend bool operator == (self_type const& L, string_t const& R)
    { return L.value == R; }
  friend bool operator != (self_type const& L, self_type const& R)
    { return L.value != R.value; }
  friend bool operator != (string_t const& L, self_type const& R)
    { return L != R.value; }
  friend bool operator != (self_type const& L, string_t const& R)
    { return L.value != R; }

  template <typename Iter>
  Iter possibly_two (Iter first, Iter last, char Next) {
    if (first != last and Next == *(first+1)) {
      this->value = string_t(first, first+2);
      ++first;
    } else
      this->value = string_t(1, *first);
    ++first;
    return first;
  }

  template <typename Iter, typename SIter>
  Iter terminal (Iter first, Iter last, SIter begin, SIter end) {
    if (first == last or begin == end) return first;
    Iter init = first;
    while (first != last and begin != end and *first == *begin)
      ++first, ++begin;
    if (begin != end)
      return init;
    this->value = string_t(init, first);
    return first;
  }

  template <typename Iter>
  Iter terminal (Iter first, Iter last, std::string const& term) {
    return this->terminal(first, last, term.begin(), term.end();
  }

  template <typename Iter>
  Iter number (Iter first, Iter last) {
    if (first == last) return first;
    Iter init = first;
    throw std::runtime_error("Digits not implemented.");
    return first;
  }

  template <typename Iter>
  Iter eat (Iter first, Iter last) {
    if (first == last) return first;
    Iter prog;
    switch (*first) {
    case '(': case ')': case '[': case ']': case '@': case ',':
    case '|': case '+': case '-': case '*': case '=': // single-char tokens
      this->value = string_t(1, *first); ++first; break;
    case '.': {
        first = this->possibly_two(first, last, '.');
      } break;
    case '/': {
        first = this->possibly_two(first, last, '/');
      } break;
    case '!': {
        first = this->possibly_two(first, last, '=');
      } break;
    case '<': {
        first = this->possibly_two(first, last, '=');
      } break;
    case '>': {
        first = this->possibly_two(first, last, '=');
      } break;
    case ':': {
        if (first != last and ':' == *(first+1)) {
          this->value = string_t(first, first+2);
          ++first;
        } else
          throw std::runtime_error("Expected another `:`.");
        ++first;
      } break;
    case '\"': {
        while (first != last and '\"' != *first)
          ++first;
        if ('\"' == *first) ++first;
        else throw std::runtime_error("Expected another `\"`.");
      } break;
    case '\'': {
        while (first != last and '\'' != *first)
          ++first;
        if ('\'' == *first) ++first;
        else throw std::runtime_error("Expected another `\'`.");
      } break;
    default: { // test the various terminals
        prog = this->terminal(first, last, "and");
        if (prog != first) return prog;
        prog = this->terminal(first, last, "or");
        if (prog != first) return prog;
        prog = this->terminal(first, last, "div");
        if (prog != first) return prog;
        prog = this->terminal(first, last, "mod");
        if (prog != first) return prog;
        prog = this->terminal(first, last, "comment");
        if (prog != first) return prog;
        prog = this->terminal(first, last, "text");
        if (prog != first) return prog;
        prog = this->terminal(first, last, "processing-instruction");
        if (prog != first) return prog;
        prog = this->terminal(first, last, "node");
        if (prog != first) return prog;
        prog = this->number(first, last);
        if (prog != first) return prog;
        prog = this->qname(first, last);
        if (prog != first) return prog;
      }
    }
    return first;
  }

  string value;
};

struct parser {
};

int main (int argc, char *argv[]) {

  return 0;
}