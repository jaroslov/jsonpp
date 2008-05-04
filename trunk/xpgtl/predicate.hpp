#ifndef XPGTL_PREDICATE
#define XPGTL_PREDICATE

namespace xpgtl {
}

#endif//XPGTL_PREDICATE

/*

template <typename Iter>
Iter relative_location_path (Iter first, Iter last) {
  return first;
}

template <typename Iter>
Iter location_path (Iter first, Iter last) {
  return first;
}

template <typename Iter>
Iter filter_expression (Iter first, Iter last) {
  return first;
}

template <typename Iter>
Iter path_expression (Iter first, Iter last) {
  if (first == last) return first;
  Iter prog = location_path(first, last);
  if (prog == first) {
    prog = filter_expression(first, last);
    if (prog == first)
      throw std::runtime_error("Nonterminating path-expression");
    first = prog;
    prog = eat_literal(first, last, "//");
    if (prog == first) {
      prog = eat_literal(first, last, "/");
      if (prog != first) {
        first = prog;
        prog = relative_location_path(first, last);
        if (prog == first)
          throw std::runtime_error("Expected relative location path.");
        first = prog;
      }
    }
  }
  return first;
}

template <typename Iter>
Iter union_expression (Iter first, Iter last) {
  if (first == last) return first;
  Iter prog = path_expression(first, last);
  if (prog == first) {
    Iter prog = union_expression(first, last);
    if (prog == first)
      throw std::runtime_error("Nonterminating union-expression");
    first = prog;
    prog = eat_literal(first, last, "|");
    if (prog == first)
      throw std::runtime_error("Expected literal `|`.");
    first = prog;
    prog = path_expression(first, last);
    if (prog == first)
      throw std::runtime_error("Expected an union-(or lower)-expression");
    first = prog;
  }
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
    first = prog;
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
    first = prog;
  }
  return first;
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
    first = prog;
  }
  return first;
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
    first = prog;
  }
  return first;
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
    first = prog;
  }
  return first;
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
    first = prog;
  }
  return first;
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
    first = prog;
  }
  return first;
}

void parse_predicate (std::string const& str) {
  or_expression(str.begin(), str.end());
}

*/
