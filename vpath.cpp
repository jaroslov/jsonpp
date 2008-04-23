#include <jsonpp.hpp>
#include <iostream>
#include <fstream>
#include <locale>
#include <boost/mpl/bool.hpp>

#include <vector>
#include <list>
#include <map>
#include <set>

/*

  Idea: provide XPATH-like functionality for boost::variant

  boost::variant
    terminals
    recursives

  V -- type of the variant

  recursive<T>::type; // returns true if the type "T" is a recursive
  std::pair<iterator,iterator>
    children (T); // sequence of children of the recursive type "T"
    N.B.: *iterator == V, i.e., the iterator dereferences to the
      global "V" type
  label tag(T); // a label for the tag
  label(const char*); // converts a c-string to a label
  label(const wchar_t*); // converts a c-wstring to a label
  label(iter,iter); // converts a sequence of a string to a label

*/

struct xpath__ {};
static const xpath__ xpath_ = xpath__();

template <typename T>
struct recursive {
  typedef boost::mpl::false_ type;
};

template <typename T, typename A>
std::string tag (std::vector<T,A> const& v, xpath__ x) {
  return "vector<"+tag(T(),x)+">";
}

template <typename T, typename A>
std::string tag (std::list<T,A> const& v, xpath__ x) {
  return "list<"+tag(T(),x)+">";
}

template <typename K, typename V, typename C, typename A>
std::string tag (std::map<K,V,C,A> const& v, xpath__ x) {
  return "map<"+tag(K(),x)+","+tag(V(),x)+">";
}

template <typename V, typename C, typename A>
std::string tag (std::set<V,C,A> const& v, xpath__ x) {
  return "set<"+tag(V(),x)+">";
}

/*template <typename Variant>
struct xpath_visitor {
  struct visitor {
    template <typename T>
    typename boost::enable_if<
      typename recursive<T,xpath>::type,
      void>::type operator () (T const& t) const {
      typedef typename iterator<T,xpath>::type xpath_iter;
      xpath_iter xtr, xnd;
      for (boost::tie(xtr,xnd)=iterator(t,xpath_); xtr!=xnd; ++xtr)
        boost::apply_visitor(*this, *xtr);
    }
    template <typename U>
    void operator () (U const& u) const {
    }
  };

  template <typename Path>
  void const_visit (Variant const& variant, Path const& path) const {
  }
  template <typename Path>
  void visit (Variant& variant, Path const& path) const {
  }
};*/

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
    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
  }


  return 0;
}
