all:
	clang++ -std=c++14 -O3 -L/usr/lib/x86_64-linux-gnu parsegraph.cc  parser.cc  ruleset.cc  yopl.cc gram.cc -lre2
lib:
	clang++ -shared -fPIC -std=c++14 -O3 -L/usr/lib/x86_64-linux-gnu parsegraph.cc  parser.cc  ruleset.cc gram.cc -lre2 -oyopl.so
