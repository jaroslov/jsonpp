all: json bel vpath

json: test_json.cpp json/*.hpp jtest

jtest: test_json.cpp json/*.hpp
	g++ -O3 -I. test_json.cpp -o jtest -Llibiconv

jsonT: test_json.cpp json/*.hpp jtest
	./jtest Examples/*.*if

belT: bel/*.hpp test_bel.cpp
	g++ -O3 -I. test_bel.cpp -o btest
	./btest

vpath: xpgtl/*.hpp test_vpath.cpp
	g++ -O3 -I. test_vpath.cpp -o vtest

xtest: xpgtl/*.hpp test_xpath.cpp
	g++ -O3 -I. test_xpath.cpp -o xtest >& error.text || open -a SubEthaEdit error.text

xpathG: xpgtl/*.hpp test_xpath.cpp
	g++ -g -I. test_xpath.cpp -o xtest

xpath: xpgtl/*.hpp test_xpath.cpp xtest

xpathT: xpgtl/*.hpp test_xpath.cpp xtest
	echo "self::array" | ./xtest examples/*.*if
	echo "self::array/string" | ./xtest examples/*.*if
	echo "//string" | ./xtest examples/*.*if

vpathT: vtest
	echo "self::array" | ./vtest examples/*.*if
	echo "self::object" | ./vtest examples/*.*if
	echo "self::object/array" | ./vtest examples/*.*if
	echo "self::array/object" | ./vtest examples/*.*if
	echo "//string" | ./vtest examples/*.*if
	echo "joins/object/a1/object/inputs/array/string" | ./vtest examples/*.*if

vpath_tests: vtest xpgtl/*.hpp test_vpath.cpp
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