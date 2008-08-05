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

tvi: test_tvi.cpp utility/*.hpp
	@g++ -O3 -I. test_tvi.cpp -o ttvi
	@./ttvi

clean:
	rm -f jtest vtest btest ttest;