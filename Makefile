all: json bel rptr tpath

json: test_json.cpp json/*.hpp jtest
	@g++ -O3 -I. test_json.cpp -o jtest -liconv
	@./jtest Examples/*.*

bel: utility/*.hpp test_bel.cpp
	@g++ -O3 -I. test_bel.cpp -o btest
	@./btest

rptr: utility/*.hpp test_regular_ptr.cpp
	@g++ -O3 -I. test_regular_ptr.cpp -o rptr
	@./rptr

tpath: test_tp.cpp treepath/*.hpp
	@g++ -O3 -I. test_tp.cpp -o ttest -liconv
	@./ttest "self::json" examples/*.*

foo:
	@./ttest "self::json/child::array" examples/*.*
	@./ttest "self::array/child::string" examples/*.*
	@./ttest "descendant-or-self::string" examples/*.*
	@./ttest "self::object" examples/*.*
	@./ttest "self::object/child::array" examples/*.*
	@./ttest "self::array/child::object" examples/*.*
	@./ttest "self::joins/child::object/child::a1/child::inputs/child::array/child::string" examples/*.*

clean:
	rm -f jtest vtest btest ttest;