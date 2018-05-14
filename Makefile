all:
	g++ -std=c++14 -L/usr/lib/x86_64-linux-gnu main.cc yopl.cc -lre2
llvm:
	g++ llvm.cc -g -lpthread  `llvm-config --cxxflags --ldflags --libs core mcjit native` -lpthread -ldl -ltinfo -lz
