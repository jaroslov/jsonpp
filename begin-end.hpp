#include <utility>

#ifndef BEGIN_END
#define BEGIN_END

namespace bel {

struct const__ {};
struct mutable__ {};
struct reverse__ {};
struct const_reverse__ {};

static const const__ const_ = const__();
static const mutable__ mutable_ = mutable__();
static const reverse__ reverse_ = reverse__();
static const const_reverse__ const_reverse_ = const_reverse__();

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

template <typename Container>
typename iterator<Container>::type begin (Container& ctr) {
  return ctr.begin();
}
template <typename Container>
typename iterator<Container,const__>::type begin (Container const& ctr) {
  return ctr.begin();
}
template <typename Container>
typename iterator<Container,const__>::type const_begin (Container& ctr) {
  return const_cast<const Container>(ctr).begin();
}
template <typename Container>
typename iterator<Container>::type end (Container& ctr) {
  return ctr.end();
}
template <typename Container>
typename iterator<Container,const__>::type end (Container const& ctr) {
  return ctr.end();
}
template <typename Container>
typename iterator<Container,const__>::type const_end (Container& ctr) {
  return const_cast<const Container>(ctr).end();
}
template <typename Container>
std::pair<typename iterator<Container>::type,
  typename iterator<Container>::type>
sequence (Container& ctr) {
  return std::make_pair(begin(ctr),end(ctr));
}
template <typename Container>
std::pair<typename iterator<Container,const__>::type,
  typename iterator<Container,const__>::type>
sequence (Container const& ctr) {
  return std::make_pair(begin(ctr),end(ctr));
}
template <typename Container>
std::pair<typename iterator<Container,const__>::type,
  typename iterator<Container,const__>::type>
const_sequence (Container& ctr) {
  return std::make_pair(const_begin(ctr),const_end(ctr));
}

template <typename Tag, typename Container>
typename iterator<Container,Tag>::type
begin (Container& ctr, Tag const& t=Tag()) {
  return ctr.begin(t);
}
template <typename Tag, typename Container>
typename iterator<Container,Tag>::type
begin (Container const& ctr, Tag const& t=Tag()) {
  return ctr.begin(t);
}
template <typename Tag, typename Container>
typename iterator<Container,Tag>::type
const_begin (Container& ctr, Tag const& t=Tag()) {
  return const_cast<const Container>(ctr).begin(t);
}
template <typename Tag, typename Container>
typename iterator<Container,Tag>::type
end (Container& ctr, Tag const& t=Tag()) {
  return ctr.end(t);
}
template <typename Tag, typename Container>
typename iterator<Container,Tag>::type
end (Container const& ctr, Tag const& t=Tag()) {
  return ctr.end(t);
}
template <typename Tag, typename Container>
typename iterator<Container,Tag>::type
const_end (Container& ctr, Tag const& t=Tag()) {
  return const_cast<const Container>(ctr).end(t);
}
template <typename Tag, typename Container>
std::pair<typename iterator<Container,Tag>::type,
  typename iterator<Container,Tag>::type>
sequence (Container& ctr, Tag const& t=Tag()) {
  return std::make_pair(begin(ctr,t),end(ctr,t));
}
template <typename Tag, typename Container>
std::pair<typename iterator<Container,Tag>::type,
  typename iterator<Container,Tag>::type>
sequence (Container const& ctr, Tag const& t=Tag()) {
  return std::make_pair(begin(ctr,t),end(ctr,t));
}
template <typename Tag, typename Container>
std::pair<typename iterator<Container,Tag>::type,
  typename iterator<Container,Tag>::type>
const_sequence (Container& ctr, Tag const& t=Tag()) {
  return std::make_pair(const_begin(ctr,t),const_end(ctr,t));
}

}

#endif//BEGIN_END
