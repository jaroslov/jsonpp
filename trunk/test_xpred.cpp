#include <xpgtl/predicate.hpp>
#include <string>
#include <stdexcept>

template <typename Iter1, typename Iter2>
Iter1 eat_literal (Iter1 first, Iter1 last, Iter2 lfirst, Iter2 llast) {
  const Iter1 orig = first;
  while (first != last and lfirst != llast and *lfirst == *first)
    ++lfirst, ++first;
  if (lfirst != llast)
    return orig;
  return first;
}

template <typename Iter>
Iter eat_literal (Iter first, Iter last, std::string const& lit) {
  return eat_literal(first, last, lit.begin(), lit.end());
}

template <typename Iter>
Iter union_expression (Iter first, Iter last) {
  return first;
}

template <typename Iter>
Iter unary_expression (Iter first, Iter last) {
  if (first == last) return first;
  Iter prog = union_expression(first, last);
  if (prog == first) {
    prog = eat_literal(first, last, "-");
    if (prog == first)
      throw std::runtime_error("Expected literal `-`.");
    first = prog;
    Iter prog = unary_expression(first, last);
    if (prog == first)
      throw std::runtime_error("Nonterminating unary-expression");
  }
  return first;
}

template <typename Iter>
Iter multiplicative_expression (Iter first, Iter last) {
  if (first == last) return first;
  Iter prog = unary_expression(first, last);
  if (prog == first) {
    Iter prog = multiplicative_expression(first, last);
    if (prog == first)
      throw std::runtime_error("Nonterminating multiplicative-expression");
    first = prog;
    prog = eat_literal(first, last, "*");
    if (prog == first) {
      prog = eat_literal(first, last, "div");
      if (prog == first) {
        prog = eat_literal(first, last, "mod");
        if (prog == first)
          throw std::runtime_error("Expected literal `*`|`div`|`mod`.");
      }
    }
    first = prog;
    prog = unary_expression(first, last);
    if (prog == first)
      throw std::runtime_error("Expected an multiplicative-(or lower)-expression");
  }
}

template <typename Iter>
Iter additive_expression (Iter first, Iter last) {
  if (first == last) return first;
  Iter prog = multiplicative_expression(first, last);
  if (prog == first) {
    Iter prog = additive_expression(first, last);
    if (prog == first)
      throw std::runtime_error("Nonterminating additive-expression");
    first = prog;
    prog = eat_literal(first, last, "+");
    if (prog == first) {
      prog = eat_literal(first, last, "-");
      if (prog == first)
        throw std::runtime_error("Expected literal `and`.");
    }
    first = prog;
    prog = multiplicative_expression(first, last);
    if (prog == first)
      throw std::runtime_error("Expected an additive-(or lower)-expression");
  }
}

template <typename Iter>
Iter relational_expression (Iter first, Iter last) {
  if (first == last) return first;
  Iter prog = additive_expression(first, last);
  if (prog == first) {
    Iter prog = relational_expression(first, last);
    if (prog == first)
      throw std::runtime_error("Nonterminating relational-expression");
    first = prog;
    prog = eat_literal(first, last, "<");
    if (prog == first) {
      prog = eat_literal(first, last, ">");
      if (prog == first) {
        prog = eat_literal(first, last, "<=");
        if (prog == first) {
          prog = eat_literal(first, last, ">=");
          if (prog == first)
            throw std::runtime_error("Expected literal `<`|`>`|`<=`|`>=`.");
        }
      }
    }
    first = prog;
    prog = additive_expression(first, last);
    if (prog == first)
      throw std::runtime_error("Expected an additive-(or lower)-expression");
  }
}

template <typename Iter>
Iter equality_expression (Iter first, Iter last) {
  if (first == last) return first;
  Iter prog = relational_expression(first, last);
  if (prog == first) {
    Iter prog = equality_expression(first, last);
    if (prog == first)
      throw std::runtime_error("Nonterminating equality-expression");
    first = prog;
    prog = eat_literal(first, last, "=");
    if (prog == first) {
      prog = eat_literal(first, last, "!=");
      if (prog == first)
        throw std::runtime_error("Expected literal `and`.");
    }
    first = prog;
    prog = relational_expression(first, last);
    if (prog == first)
      throw std::runtime_error("Expected an equality-(or lower)-expression");
  }
}

template <typename Iter>
Iter and_expression (Iter first, Iter last) {
  if (first == last) return first;
  Iter prog = equality_expression(first, last);
  if (prog == first) {
    Iter prog = and_expression(first, last);
    if (prog == first)
      throw std::runtime_error("Nonterminating and-expression");
    first = prog;
    prog = eat_literal(first, last, "and");
    if (prog == first)
      throw std::runtime_error("Expected literal `and`.");
    first = prog;
    prog = equality_expression(first, last);
    if (prog == first)
      throw std::runtime_error("Expected an equality-(or lower)-expression");
  }
}

template <typename Iter>
Iter or_expression (Iter first, Iter last) {
  if (first == last) return first;
  Iter prog = and_expression(first, last);
  if (prog == first) {
    Iter prog = or_expression(first, last);
    if (prog == first)
      throw std::runtime_error("Nonterminating or-expression");
    first = prog;
    prog = eat_literal(first, last, "or");
    if (prog == first)
      throw std::runtime_error("Expected literal `or`.");
    first = prog;
    prog = and_expression(first, last);
    if (prog == first)
      throw std::runtime_error("Expected an and-(or lower)-expression");
  }
}

void parse_predicate (std::string const& str) {
  or_expression(str.begin(), str.end());
}

int main (int argc, char *argv[]) {

  return 0;
}