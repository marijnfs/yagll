#include "yopl.h"

using namespace std;

int main(int argc, char **argv) {
  cout << "yopl" << endl;
  if (argc < 3) {
    cerr << "not enough inputs, use *prog* [gram] [input]" << endl;
    return 1;
  }
  
  string gram_file(argv[1]);
  string input_file(argv[2]);
  
  Parser parser(gram_file);
  auto parse_result = parser.parse(input_file);
  
  
}
