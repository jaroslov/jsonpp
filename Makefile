all: json

json: jsonpp.cpp jsonpp.hpp
	g++ -O3 -I. jsonpp.cpp -o jsonpp

test_json: jsonpp jsonpp.hpp jsonpp.cpp
	./jsonpp Examples/*.*if

bel: begin-end.hpp bel.cpp
	g++ -O3 -I. bel.cpp -o bel

test_bel: bel begin-end.hpp bel.cpp
	./bel

clean: jsonpp
	rm jsonpp