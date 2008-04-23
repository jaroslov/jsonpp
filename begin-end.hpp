#include <utility>
#include <boost/mpl/vector.hpp>
#ifdef BEL_IOSTREAM
#include <iostream>
#include <iterator>
#endif

#ifndef BEGIN_END
#define BEGIN_END

namespace bel {

// these are the standard library iterator kind tags, i.e.,
// the kinds of iterators that the containers in the STL provide
struct const__ {};
struct mutable__ {};
struct reverse__ {};
struct paired__ {};

static const const__ const_ = const__();
static const mutable__ mutable_ = mutable__();
static const reverse__ reverse_ = reverse__();
static const paired__ paired_ = paired__();

// composite
typedef boost::mpl::vector<const__,reverse__> const_reverse__;
static const const_reverse__ const_reverse_ = const_reverse__();

// these are the standard iterator "tag class" kind tags, i.e.,
// the semantic properties rather than syntactic properties supported by the
// STL
struct trivial__ {};
struct input__ {};
struct output__ {};
struct forward__ {};
struct bidirection__ {};
struct random_access__ {};

static const trivial__ trivial_ = trivial__();
static const input__ input_ = input__();
static const output__ output_ = output__();
static const forward__ forward_ = forward__();
static const bidirection__ bidirection_ = bidirection__();
static const random_access__ random_access_ = random_access__();

template <typename Container, typename Tag=void>
struct iterator {
  typedef typename Container::iterator type;
};
template <typename Container>
struct iterator<Container,const__> {
  typedef typename Container::const_iterator type;
};
template <typename Container>
struct iterator<Container,reverse__> {
  typedef typename Container::reverse_iterator type;
};
template <typename Container>
struct iterator<Container,const_reverse__> {
  typedef typename Container::const_reverse_iterator type;
};

/// [STL Tag Support] ======
//      properly dispatch to the "named" begin/end function
template <typename Container>
typename iterator<Container>::type
begin (Container& ctr, mutable__ const& m=mutable__()) {
  return ctr.begin();
}
template <typename Container>
typename iterator<Container,const__>::type
begin (Container const& ctr, const__ const& c=const__()) {
  return ctr.begin();
}
template <typename Container>
typename iterator<Container,const__>::type
begin (Container& ctr, const__) {
  return const_cast<const Container*>(&ctr)->begin();
}
template <typename Container>
typename iterator<Container,reverse__>::type
begin (Container& ctr, reverse__) {
  return ctr.rbegin();
}
template <typename Container>
typename iterator<Container,const_reverse__>::type
begin (Container const& ctr, const_reverse__) {
  return ctr.rbegin();
}
template <typename Container>
typename iterator<Container,const_reverse__>::type
begin (Container& ctr, const_reverse__) {
  return const_cast<const Container*>(&ctr)->rbegin();
}

template <typename Container>
typename iterator<Container>::type
end (Container& ctr, mutable__ const& m=mutable__()) {
  return ctr.end();
}
template <typename Container>
typename iterator<Container,const__>::type
end (Container const& ctr, const__ const& c=const__()) {
  return ctr.end();
}
template <typename Container>
typename iterator<Container,const__>::type
end (Container& ctr, const__) {
  return const_cast<const Container*>(&ctr)->end();
}
template <typename Container>
typename iterator<Container,reverse__>::type
end (Container& ctr, reverse__) {
  return ctr.rend();
}
template <typename Container>
typename iterator<Container,const_reverse__>::type
end (Container const& ctr, const_reverse__) {
  return ctr.rend();
}
template <typename Container>
typename iterator<Container,const_reverse__>::type
end (Container& ctr, const_reverse__) {
  return const_cast<const Container*>(&ctr)->rend();
}

template <typename Container>
std::pair<typename iterator<Container>::type,
  typename iterator<Container>::type>
sequence (Container& ctr, mutable__ const& m=mutable__()) {
  return std::make_pair(bel::begin(ctr),bel::end(ctr));
}
template <typename Container>
std::pair<typename iterator<Container,const__>::type,
  typename iterator<Container,const__>::type>
sequence (Container const& ctr, const__ const& c=const__()) {
  return std::make_pair(bel::begin(ctr),bel::end(ctr));
}
template <typename Container>
std::pair<typename iterator<Container,const__>::type,
  typename iterator<Container,const__>::type>
sequence (Container& ctr, const__ const& c) {
  return std::make_pair(bel::begin(ctr,c),bel::end(ctr,c));
}
template <typename Container>
std::pair<typename iterator<Container,reverse__>::type,
  typename iterator<Container,reverse__>::type>
sequence (Container& ctr, reverse__ const& r) {
  return std::make_pair(bel::begin(ctr,r),bel::end(ctr,r));
}
template <typename Container>
std::pair<typename iterator<Container,const_reverse__>::type,
  typename iterator<Container,const_reverse__>::type>
sequence (Container const& ctr, const_reverse__ const& r) {
  return std::make_pair(bel::begin(ctr,r),bel::end(ctr,r));
}
template <typename Container>
std::pair<typename iterator<Container,const_reverse__>::type,
  typename iterator<Container,const_reverse__>::type>
sequence (Container& ctr, const_reverse__ const& r) {
  return std::make_pair(bel::begin(ctr,r),bel::end(ctr,r));
}

#ifdef BEL_IOSTREAM

#ifdef CPP0x
#define BEL_EQ_CHART =CharT
#else
#define BEL_EQ_CHART
#endif
template <typename CharT, typename CharS BEL_EQ_CHART>
std::istream_iterator<CharT,CharS>
begin (std::basic_istream<CharT>& bistr, forward__ const& f=forward__()) {
  return std::istream_iterator<CharT,CharS>(bistr);
}
template <typename CharT, typename CharS BEL_EQ_CHART>
std::istream_iterator<CharT,CharS>
end (std::basic_istream<CharT>&, forward__ const& f=forward__()) {
  return std::istream_iterator<CharT,CharS>();
}
template <typename CharT, typename CharS BEL_EQ_CHART>
std::pair<std::istream_iterator<CharT,CharS>,
  std::istream_iterator<CharT,CharS> >
sequence (std::basic_istream<CharT>& bistr, forward__ const& f=forward__()) {
  return std::make_pair(bel::begin(bistr,f),bel::end(bistr,f));
}

template <typename CharT, typename CharS BEL_EQ_CHART>
std::ostream_iterator<CharT,CharS>
begin (std::basic_ostream<CharT>& bistr, forward__ const& f=forward__()) {
  return std::ostream_iterator<CharT,CharS>(bistr);
}
template <typename CharT, typename CharS BEL_EQ_CHART>
std::ostream_iterator<CharT,CharS>
end (std::basic_ostream<CharT>&, forward__ const& f=forward__()) {
  return std::ostream_iterator<CharT,CharS>();
}
template <typename CharT, typename CharS BEL_EQ_CHART>
std::pair<std::ostream_iterator<CharT,CharS>,
  std::ostream_iterator<CharT,CharS> >
sequence (std::basic_ostream<CharT>& bistr, forward__ const& f=forward__()) {
  return std::make_pair(bel::begin(bistr,f),bel::end(bistr,f));
}

#endif

/// [Generalized Tag Support] ======
//    the following functions support containers which are iterator-tag
//    aware:
//        C -- a container
//        t -- a tag
//      Valid Expressions:
//        C.begin(t);
//        C.end(t);
template <typename Tag, typename Container>
typename iterator<Container,Tag>::type
begin (Container& ctr, Tag const& t=Tag()) {
  return ctr.begin();
}
template <typename Tag, typename Container>
typename iterator<Container,Tag>::type
begin (Container const& ctr, Tag const& t=Tag()) {
  return ctr.begin();
}

template <typename Tag, typename Container>
typename iterator<Container,Tag>::type
end (Container& ctr, Tag const& t=Tag()) {
  return ctr.end();
}
template <typename Tag, typename Container>
typename iterator<Container,Tag>::type
end (Container const& ctr, Tag const& t=Tag()) {
  return ctr.end();
}

template <typename Tag, typename Container>
std::pair<typename iterator<Container,Tag>::type,
  typename iterator<Container,Tag>::type>
sequence (Container& ctr, Tag const& t=Tag()) {
  return std::make_pair(bel::begin(ctr,t),bel::end(ctr,t));
}
template <typename Tag, typename Container>
std::pair<typename iterator<Container,Tag>::type,
  typename iterator<Container,Tag>::type>
sequence (Container const& ctr, Tag const& t=Tag()) {
  return std::make_pair(bel::begin(ctr,t),bel::end(ctr,t));
}

}

#endif//BEGIN_END
