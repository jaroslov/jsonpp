#include <iostream>
#include <vector>
#include <regular_ptr/regular_ptr.hpp>

class A_base {
public:
	virtual int foo () const = 0;
};
template <int K>
class A : public A_base {
public:
	virtual int foo () const { return K; }
};
class G : public A_base {
public:
	G () : value(0) {}
	G (int v) : value(v) {}
	virtual int foo () const { return this->value; }
	int value;
};

template <int S>
bool operator == (A<S>, A<S>) {
	return true;
}
template <int S>
bool operator != (A<S>, A<S>) {
	return false;
}

bool operator == (G const& left, G const& right) { return left.value == right.value; }
bool operator != (G const& left, G const& right) { return left.value != right.value; }

void test_regular () {
	typedef jsonpp::regular_ptr<A_base> a_ptr;

	a_ptr rptr1, rptr2;
	rptr1 = A<5>();
	rptr2 = A<6>();

	std::cout << rptr1->foo() << " " << rptr2->foo() << " " << (rptr1 == rptr2) << std::endl;

	swap(rptr1, rptr2);
	std::cout << rptr1->foo() << " " << rptr2->foo() << " " << (rptr1 == rptr2) << std::endl;

	rptr1 = rptr2;
	std::cout << rptr1->foo() << " " << rptr2->foo() << " " << (rptr1 == rptr2) << std::endl;

	std::vector<a_ptr> V;
	V.push_back(a_ptr(A<1>()));
	V.push_back(a_ptr(A<2>()));
	V.push_back(a_ptr(A<3>()));
	V.push_back(a_ptr(A<4>()));
	V.push_back(a_ptr(A<5>()));
	V.push_back(a_ptr(A<6>()));
	for (std::size_t i=0; i<10; ++i)
		V.push_back(a_ptr(G(i)));

	for (std::size_t i=0; i<V.size(); ++i)
		std::cout << V[i]->foo() << " ";
	std::cout << std::endl;
	std::reverse(V.begin(), V.end());
	for (std::size_t i=0; i<V.size(); ++i)
		std::cout << V[i]->foo() << " ";
	std::cout << std::endl;


	while (not V.empty()) {
		std::cout << "Countdown: " << V.size() << std::endl;
		V.pop_back();
	}
}


int main (int argc, char *argv[]) {

	test_regular();

	return 0;
}
