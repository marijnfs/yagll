all:
	g++ -std=c++14 -g -o yopl *.cc
fast:
	g++ -std=c++14 -O3 -o yopl *.cc
