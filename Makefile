all:
	clang++ -shared -fPIC -std=c++17 -L/usr/lib/x86_64-linux-gnu parsegraph.cc  parser.cc  ruleset.cc gram.cc searchnode.cc -lre2 -olibyagll.so
	#clang++ -std=c++17 -L/usr/lib/x86_64-linux-gnu parse.cc -oparse -lre2 -L. -lyagll -lstdc++fs
