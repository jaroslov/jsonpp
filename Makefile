
all: json bel vpath

json: jsonpp.cpp jsonpp.hpp
	g++ -O3 -I. jsonpp.cpp -o jsonpp

test_json: jsonpp jsonpp.hpp jsonpp.cpp
	./jsonpp Examples/*.*if

bel: begin-end.hpp bel.cpp
	g++ -O3 -I. bel.cpp -o bel

test_bel: bel begin-end.hpp bel.cpp
	./bel

vpath: xpath/*.hpp vpath.cpp
	g++ -O3 -I. vpath.cpp -o vpath

test_vpath: vpath xpath/*.hpp vpath.cpp
	echo "$(PROG_ARG)" | ./vpath Examples/*.*if

vpatht: vpath xpath/*.hpp vpath.cpp
	echo "/object/array" | ./vpath Examples/*.*if
	echo "/array/object" | ./vpath Examples/*.*if

vpath_tests: vpath xpath/*.hpp vpath.cpp
	echo "/" | ./vpath
	echo "foo" | ./vpath
	echo "foo[0]" | ./vpath
	echo "parent::node()[0]" | ./vpath
	echo "/foo/bar" | ./vpath
	echo "/foo/bar[0]" | ./vpath
	echo "/foo/*[0]" | ./vpath
	echo "/foo/@*[0]/text()" | ./vpath
	echo "//foo[0]/.[10]/ancestor-or-self/node()" | ./vpath
	echo "/..[1]/child::bar[0]/&lt;child&gt;" | ./vpath

clean:
	rm jsonpp;
	rm vpath;
	rm bel;