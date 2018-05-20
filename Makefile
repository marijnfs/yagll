all:
	clang++ -std=c++14 -g -L/usr/lib/x86_64-linux-gnu parsegraph.cc  parser.cc  ruleset.cc  yopl.cc -lre2
llvm:
	g++ llvm.cc -g -lpthread  `llvm-config --cxxflags --ldflags --libs core mcjit native` -lpthread -ldl -ltinfo -lz
