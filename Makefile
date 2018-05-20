all:
	clang++ -std=c++14 -g -L/usr/lib/x86_64-linux-gnu main.cc yopl.cc ruleset.cc -lre2
llvm:
	g++ llvm.cc -g -lpthread  `llvm-config --cxxflags --ldflags --libs core mcjit native` -lpthread -ldl -ltinfo -lz
