#include "yagll.h"

#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
  if (argc < 3) {
    cout << argv[0] << " [gramfile] [parsefile]" << endl;
    return -1;
  }

  ifstream grammar_file(argv[1]);
  ifstream input_file(argv[2]);
  Parser parser(grammar_file);
  auto pg = parser.parse(input_file);
  pg->pprint(cout);
  return 0;
}
