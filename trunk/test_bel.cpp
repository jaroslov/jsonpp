#define BEL_IOSTREAM
#include <bel/begin-end.hpp>
#include <vector>
#include <list>
#include <map>
#include <set>

template <typename Ctr>
void test_stl (Ctr& ctr) {
  // non-const
  bel::begin(ctr); bel::end(ctr); bel::sequence(ctr);
  // const
  Ctr const& rctr = *const_cast<const Ctr*>(&ctr);
  bel::begin(rctr); bel::end(rctr); bel::sequence(rctr);
  bel::begin(ctr,bel::const_); bel::end(ctr,bel::const_);
    bel::sequence(ctr,bel::const_);
  // reverse
  bel::begin(ctr,bel::reverse_); bel::end(ctr,bel::reverse_);
    bel::sequence(ctr,bel::reverse_);
  bel::begin(rctr,bel::const_reverse_);
    bel::end(rctr,bel::const_reverse_);
    bel::sequence(rctr,bel::const_reverse_);
  bel::begin(ctr,bel::const_reverse_);
    bel::end(ctr,bel::const_reverse_);
    bel::sequence(ctr,bel::const_reverse_);
}

int main (int argc, char *argv[]) {

  std::vector<int> V;
  std::list<int> L;
  std::map<int,int> M;
  std::set<int> S;

  test_stl(V);
  test_stl(L);
  test_stl(M);
  test_stl(S);

  return 0;
}
