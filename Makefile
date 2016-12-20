all:
	g++ -std=c++14 -O3 -lre2 yopl.cc
llvm:
	g++ llvm.cc -lpthread  `llvm-config --cxxflags --ldflags --libs all` -lpthread -ldl -ltinfo
