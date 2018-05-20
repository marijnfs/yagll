#include "parser.h"

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
  auto parse_graph = parser.parse(input_file);

  if (!parse_graph)
    return 0;

  cout << "nodes: " << parse_graph->size() << endl;
  parse_graph->print_dot("parse.dot");

  parse_graph->filter([](ParseGraph &pg, int n) {
    if (pg.name_ids[n] == -1)
      pg.cleanup[n] = true;
    if (pg.name_map[pg.name_ids[n]] == "ws")
      pg.cleanup[n] = true;
  });
  parse_graph->compact();
  parse_graph->print_dot("compact.dot");

  for (int n(0); n < parse_graph->nodes.size(); ++n) {
    if (parse_graph->name_ids[n] >= 0) {
      string sub = parse_graph->ends[n] < 0
                       ? "NEG"
                       : parser.buffer.substr(parse_graph->starts[n],
                                              parse_graph->ends[n] -
                                                  parse_graph->starts[n]);
      cout << parse_graph->name_map[parse_graph->name_ids[n]] << " " << sub
           << " " << parse_graph->starts[n] << endl;
    }
  }
}
