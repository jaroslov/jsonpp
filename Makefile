all: json bel vpath

json: jsonpp.cpp jsonpp.hpp
	g++ -O3 -I. jsonpp.cpp -o jsonpp

test_json: jsonpp jsonpp.hpp jsonpp.cpp
	./jsonpp Examples/*.*if

bel: begin-end.hpp bel.cpp
	g++ -O3 -I. bel.cpp -o bel

test_bel: bel begin-end.hpp bel.cpp
	./bel

vpath: path.hpp vpath.cpp
	g++ -O3 -I. vpath.cpp -o vpath

test_vpath: vpath path.hpp vpath.cpp
	./vpath Examples/*.*if

clean:
	rm jsonpp;
	rm vpath;
	rm bel;