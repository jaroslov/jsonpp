all: json bel vpath

jsonT: test_json.cpp json/*.hpp
	g++ -O3 -I. test_json.cpp -o jtest
	./jtest Examples/*.*if

belT: bel/*.hpp test_bel.cpp
	g++ -O3 -I. test_bel.cpp -o btest
	./btest

vpathT: xpath/*.hpp test_vpath.cpp
	g++ -O3 -I. test_vpath.cpp -o vtest
	echo "/object/array" | ./vtest Examples/*.*if
	echo "/array/object" | ./vtest Examples/*.*if

vpath_tests: vtest xpath/*.hpp test_vpath.cpp
	echo "/" | ./vtest
	echo "foo" | ./vtest
	echo "foo[0]" | ./vtest
	echo "parent::node()[0]" | ./vtest
	echo ".[0]" | ./vtest
	echo "/foo/bar/.././@blah" | ./vtest
	echo "/foo/bar" | ./vtest
	echo "/foo/bar[0]" | ./vtest
	echo "/foo/*[0]" | ./vtest
	echo "/foo/@*[0]/text()" | ./vtest
	echo "//foo[0]/.[10]/ancestor-or-self/node()" | ./vtest
	echo "/..[1]/child::bar[0]/&lt;child&gt;" | ./vtest

clean:
	rm jtest vtest btest;