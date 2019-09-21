#include "yagll.h"

#include <fstream>
#include <iostream>
#include <string>
#include <experimental/filesystem>

using namespace std;

namespace fs = std::experimental::filesystem;

int main(int argc, char **argv) {
  cout << "yopl" << endl;
  if (argc < 3) {
    cerr << "not enough inputs, use: " << argv[0] << " [gram] [input]" << endl;
    return 1;
  }

  fs::path gram_path(argv[1]);
  fs::path input_path(argv[2]);

  if (!fs::exists(gram_path))
    throw StringException("Grammar file doesn't exist");
  if (!fs::exists(input_path))
    throw StringException("Input file doesn't exist");

  for (auto file : fs::directory_iterator(input_path.parent_path())) {
    cout << file << endl;
  }

  ifstream gram_file(gram_path);
  ifstream input_file(input_path);

  Parser parser(gram_file);
  auto parse_graph = parser.parse(input_file);
  
  if (!parse_graph)
    return 0;

  tree_print(*parse_graph, 0, 0);
}
