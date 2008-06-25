#include <iostream>
#include <vector>
#include <valued_interface_pt/valued_interface_ptr.hpp>

struct foo_base {
  virtual int foo () const = 0;
};
template <int K>
struct foo_t : foo_base {
  virtual int foo () const { return K; }
  int bar () { return K*K; }
};

int main (int argc, char *argv[]) {

  typedef valued_interface_ptr<foo_base> foo_base_ptr;
  typedef std::vector<foo_base_ptr> v_foo_base_ptrs;

  v_foo_base_ptrs buffer;
  for (std::size_t i=0; i<100; ++i) {
    buffer.push_back(foo_base_ptr(foo_t<0>()));
    buffer.push_back(foo_base_ptr(foo_t<1>()));
    buffer.push_back(foo_base_ptr(foo_t<2>()));
    buffer.push_back(foo_base_ptr(foo_t<3>()));
    buffer.push_back(foo_base_ptr(foo_t<4>()));
  }
  for (std::size_t i=0; i<155; ++i)
    buffer.pop_back();

  for (std::size_t i=0; i<buffer.size(); ++i)
    if (not buffer[i].empty())
      std::cout << buffer[i]->foo() << " ";
  std::cout << std::endl;

  return 0;
}
