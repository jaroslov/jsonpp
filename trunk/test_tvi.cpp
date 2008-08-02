#include <iostream>
#include <iomanip>
#include <utility/iterator.hpp>

int main (int argc, char *argv[]) {

	utility::trivial_value_iterator<int> iter1(1), iter2(2), iter3(3), end;

	std::cout << *iter1 << " " << *iter2 << " " << *iter3 << std::endl;

	std::cout << std::boolalpha << (iter1 == iter2) << " "
						<< (iter1 == iter1) << " " << (iter3 == end)
						<< " " << (end == end) << std::endl;

	for ( ; iter1 != end; ++iter1)
		std::cout << *iter1 << std::endl;

	return 0;
}
