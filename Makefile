all:
	g++ -std=c++14 -lre2 -g -o yopl yopl.cc
fast:
	g++ -std=c++14 -lre2 -O3 -o yopl yopl.cc
profile:
	g++ -std=c++14 -lre2 -pg -o yopl yopl.cc
