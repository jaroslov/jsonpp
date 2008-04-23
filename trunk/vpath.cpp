#include <jsonpp.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>
#include <boost/mpl/bool.hpp>
#include <boost/tuple/tuple.hpp>

#include <vector>
#include <list>
#include <map>
#include <set>
#include <string>

/*

  Idea: provide XPATH-like functionality for boost::variant

  boost::variant
    terminals
    recursives

  V -- type of the variant

*/

struct xpath__ {};
static const xpath__ xpath_ = xpath__();

template <typename T>
struct recursive : public boost::mpl::false_ {};

template <typename T, typename A>
struct recursive<std::vector<T,A> > : public boost::mpl::true_ {};
template <typename T, typename A>
struct recursive<std::list<T,A> > : public boost::mpl::true_ {};
template <typename K, typename V, typename C, typename A>
struct recursive<std::map<K,V,C,A> > : public boost::mpl::true_ {};
template <typename V, typename C, typename A>
struct recursive<std::set<V,C,A> > : public boost::mpl::true_ {};

std::string name (bool, xpath__) { return "bool"; }
std::string name (char, xpath__) { return "char"; }
std::string name (wchar_t, xpath__) { return "wchar_t"; }
std::string name (signed short, xpath__) { return "sshort"; }
std::string name (unsigned short, xpath__) { return "ushort"; }
std::string name (signed int, xpath__) { return "sint"; }
std::string name (unsigned int, xpath__) { return "uint"; }
std::string name (signed long, xpath__) { return "slong"; }
std::string name (unsigned long, xpath__) { return "ulong"; }
std::string name (float, xpath__) { return "float"; }
std::string name (double, xpath__) { return "double"; }

template <typename T>
std::string name (std::basic_string<T> const& str, xpath__ x) {
  return "string<"+name(T(),x)+">";
}
template <typename T, typename A>
std::string name (std::vector<T,A> const& v, xpath__ x) {
  return "vector<"+name(T(),x)+">";
}
template <typename T, typename A>
std::string name (std::list<T,A> const& v, xpath__ x) {
  return "list<"+name(T(),x)+">";
}
template <typename K, typename V, typename C, typename A>
std::string name (std::map<K,V,C,A> const& v, xpath__ x) {
  return "map<"+name(K(),x)+","+name(V(),x)+">";
}
template <typename V, typename C, typename A>
std::string name (std::set<V,C,A> const& v, xpath__ x) {
  return "set<"+name(V(),x)+">";
}

template <typename Value, typename Iterator>
struct get_second_facade {
  Iterator iterator_;

  get_second_facade (Iterator const& iter=Iterator()) : iterator_(iter) {}
  get_second_facade (get_second_facade const& gsf) : iterator_(gsf.iterator_) {}
  get_second_facade& operator = (get_second_facade const& gsf) {
    this->iterator_ = gsf.iterator_;
    return *this;
  }

  friend bool
  operator == (get_second_facade const& L, get_second_facade const& R) {
    return L.iterator_ == R.iterator_;
  }
  friend bool
  operator != (get_second_facade const& L, get_second_facade const& R) {
    return L.iterator_ != R.iterator_;
  }

  Value const& operator * () const {
    return this->iterator_->second;
  }
  const Value* operator -> () const {
    return &this->iterator_->second;
  }
  get_second_facade& operator ++ () {
    ++this->iterator_; return *this;
  }
  get_second_facade operator ++ (int) {
    get_second_facade cp(*this);
    ++*this;
    return cp;
  }
};

namespace bel {

template <typename T, typename A>
struct iterator<std::vector<T,A>,xpath__> {
  typedef typename std::vector<T,A>::const_iterator type;
};
template <typename T, typename A>
struct iterator<std::list<T,A>,xpath__> {
  typedef typename std::list<T,A>::const_iterator type;
};
template <typename K, typename V, typename C, typename A>
struct iterator<std::map<K,V,C,A>,xpath__> {
  typedef get_second_facade<V,typename std::map<K,V,C,A>::const_iterator> type;
};
template <typename V, typename C, typename A>
struct iterator<std::set<V,C,A>,xpath__> {
  typedef typename std::set<V,C,A>::const_iterator type;
};

template <typename V, typename K, typename C, typename A>
std::pair<typename iterator<std::map<V,K,C,A>,xpath__>::type,
  typename iterator<std::map<V,K,C,A>,xpath__>::type>
sequence (std::map<V,K,C,A> const& m, xpath__) {
  typedef typename iterator<std::map<V,K,C,A>,xpath__>::type iter_t;
  return std::make_pair(iter_t(m.begin()),iter_t(m.end()));
}

}

template <typename Variant>
struct query_generator {
  enum type_e {
    ancestor,
    ancestor_or_self,
    attribute,
    child,
    descendent,
    descendent_or_self,
    following,
    following_sibling,
    namespace_,
    parent,
    preceding,
    preceding_sibling,
    self,
  };

  template <typename Iter>
  void lex (Iter first, Iter second) const {
    /*
      Our "generalized" xpath language is this:
      
      Start ::= AbsPath 
              | RelPathS
      AbsPath ::= / RelPathS
      RelPathS ::= RelPath
                | RelPath / RelPathS
      RelPath ::= Step
                | Step Predicate
                | Step Attribute
      Step ::= Identifier
             | .
             | ..
      Predicate ::= [ $ Number ]
      Attribute ::= @ Number
    */
    switch (*first) {
    case '[' : break;
    case ']' : break;
    case '(' : break;
    case ')' : break;
    case '$' : break;
    case '/' : break;
    case ':' : break;
    case '@' : break;
    case '.' : break;
    default : break;
    }
  }

  struct visitor : boost::static_visitor<> {
    std::string depth;

    visitor (std::string const& depth="") : depth(depth) {}

    template <typename T>
    typename boost::enable_if<recursive<T> >::type
    operator () (T const& t) const {
      typedef typename bel::iterator<T,xpath__>::type iterator;
      std::cout << this->depth << name(t, xpath_) << std::endl;
      visitor V(this->depth+"  ");
      iterator itr, ind;
      for (boost::tie(itr,ind)=bel::sequence(t, xpath_); itr!=ind; ++itr)
        boost::apply_visitor(V, *itr);
    }
    template <typename T>
    typename boost::disable_if<recursive<T> >::type
    operator () (T const& t) const {
      std::cout << this->depth << name(t, xpath_) << std::endl;
    }
  };

  template <typename Path>
  void operator () (Variant const& variant, Path const& path) const {
    // for now, assume that Path is a std::string
    //
    visitor V;
    boost::apply_visitor(V, variant);
  }
};

template <typename Variant, typename Path>
void query (Variant const& variant, Path const& path) {
  query_generator<Variant> Qg;
  Qg(variant, path);
}

std::string name (JSONpp::nil, xpath__) {
  return "nil";
}
std::string name (JSONpp::json_v, xpath__) {
  return "json";
}

int main (int argc, char *argv[]) {

  std::locale loc("");
  std::wcout.imbue(loc);

  if (argc < 1)
    return 1;

  for (++argv; argc > 0; --argc, ++argv) {
    std::cout << *argv << std::endl;
    try {
      std::wifstream wifstr(*argv);
      wifstr.imbue(loc);
      wifstr >> std::noskipws;
      std::istream_iterator<wchar_t,wchar_t> ctr(wifstr);
      std::istream_iterator<wchar_t,wchar_t> cnd;
      JSONpp::json_v json = JSONpp::parse(ctr, cnd);
      query(json, "/joins/inputs");
    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
  }


  return 0;
}
