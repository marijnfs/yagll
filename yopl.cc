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

  cout << "BFS:" << endl;
  parse_graph->visit_bfs([](ParseGraph &pg, int n){
      cout << pg.name(n) << " - " << pg.substr(n) << endl;
    });
  cout << endl;

  cout << "DFS:" << endl;
  parse_graph->visit_dfs([](ParseGraph &pg, int n){
      cout << pg.name(n) << " - " << pg.substr(n) << endl;
    });
  cout << endl;


  cout << "LEAF:" << endl;
  vector<int> bla(parse_graph->size());
  parse_graph->visit_bottom_up([&bla](ParseGraph &pg, int n){
      cout << "{" << pg.name(n) << " : " << pg.substr(n) << endl;
      bla[n] = 1;
      for (int c : pg.nodes[n].children) {
        cout << "}" << pg.name(c) << " : " << pg.substr(c) << endl;
        if (bla[c] != 1)
          throw "";
      }
    });

  
}
