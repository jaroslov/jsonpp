all: json bel vpath

json: jsonpp.cpp jsonpp.hpp
	g++ -O3 -I. jsonpp.cpp -o jsonpp

test_json: jsonpp jsonpp.hpp jsonpp.cpp
	./jsonpp Examples/*.*if

bel: begin-end.hpp bel.cpp
	g++ -O3 -I. bel.cpp -o bel

test_bel: bel begin-end.hpp bel.cpp
	./bel

vpath: xpath/path.hpp xpath/query.hpp vpath.cpp
	g++ -O3 -I. vpath.cpp -o vpath

test_vpath: vpath xpath/path.hpp xpath/query.hpp vpath.cpp
	echo "$(PROG_ARG)" | ./vpath Examples/*.*if

vpath_tests: vpath xpath/path.hpp xpath/query.hpp vpath.cpp
	echo "/foo/bar" | ./vpath
	echo "/foo/bar[0]" | ./vpath
	echo "/foo/*[0]" | ./vpath
	echo "/foo/@*[0]" | ./vpath
	echo "//child::foo[0]" | ./vpath
	echo "/..[1]/child::bar[0]" | ./vpath

clean:
	rm jsonpp;
	rm vpath;
	rm bel;