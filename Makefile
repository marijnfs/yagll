all:
	g++ -std=c++14 -O3 -lre2 yopl.cc
llvm:
	g++ llvm.cc -g -lpthread  `llvm-config --cxxflags --ldflags --libs core mcjit native` -lpthread -ldl -ltinfo -lz
