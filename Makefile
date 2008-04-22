all: json

json: jsonpp.cpp jsonpp.hpp
	g++ -O3 -I. jsonpp.cpp -o jsonpp

test: jsonpp jsonpp.hpp jsonpp.cpp
	./jsonpp Examples/*.*if

clean: jsonpp
	rm jsonpp