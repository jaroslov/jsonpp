#include <lstring/lstring.hpp>

/*
	
	Logical String:

	Essentially a type of `DAG'y string:

	list
	  | chunks

	chunk
	  | string
		| reference-to-chunk
		| list-of-chunks
*/

int main (int argc, char *argv[]) {

	lstring::lstring<> L;

	return 0;
}
