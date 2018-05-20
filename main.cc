#include "yopl.h"

using namespace std;

int main(int argc, char **argv) {
  cout << "yopl" << endl;
  if (argc < 3) {
    cerr << "not enough inputs, use: " << argv[0] << " [gram] [input]" << endl;
    return 1;
  }

  string gram_file(argv[1]);
  string input_file(argv[2]);

  Parser parser(gram_file);
  auto parse_result = parser.parse(input_file);
  cout << "nodes: " << parse_result->nodes.size() << endl;
  parse_result->print_dot("parse.dot");
  parse_result->filter([](ParseGraph &pg, int n) {
    if (pg.name_ids[n] == -1)
      pg.cleanup[n] = true;
    if (pg.name_map[pg.name_ids[n]] == "ws")
      pg.cleanup[n] = true;
  });
  parse_result->compact();
  parse_result->print_dot("compact.dot");

  for (int n(0); n < parse_result->nodes.size(); ++n) {
    if (parse_result->name_ids[n] >= 0) {
      string sub = parse_result->ends[n] < 0
                       ? "NEG"
                       : parser.buffer.substr(parse_result->starts[n],
                                              parse_result->ends[n] -
                                                  parse_result->starts[n]);
      cout << parse_result->name_map[parse_result->name_ids[n]] << " " << sub
           << " " << parse_result->starts[n] << endl;
    }
  }
}
