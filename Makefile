all:
	clang++ -std=c++14 -O3 -L/usr/lib/x86_64-linux-gnu parsegraph.cc  parser.cc  ruleset.cc  yopl.cc gram.cc -lre2
llvm:
	g++ llvm.cc -g -lpthread  `llvm-config --cxxflags --ldflags --libs core mcjit native` -ldl -ltinfo -lz
