#include "parser.h"
#include "gram.h"

using namespace std;

int main(int argc, char **argv) {
  cout << "yopl" << endl;
  if (argc < 3) {
    cerr << "not enough inputs, use: " << argv[0] << " [gram] [input]" << endl;
    return 1;
  }

  string gram_file(argv[1]);
  string input_file(argv[2]);

  ifstream gram_in(gram_file);
  Parser parser(gram_in);
  ifstream input_in(input_file);
  auto parse_graph = parser.parse(input_in);
  
  //parser.dot_graph_debug("graphdebug.dot");
  
  if (!parse_graph)
    return 0;

  cout << "nodes: " << parse_graph->size() << endl;
  parse_graph->print_dot("compact.dot");

  /*
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
    }*/

  cout << "BFS:" << endl;
  auto statements = parse_graph->get_connected(0, "entry");
  for (auto s : statements) {
    cout << s << endl;
  }

  parse_graph->visit_bfs(0, [](ParseGraph &pg, int n){
      //cout << pg.name(n) << " - " << pg.substr(n) << endl;
    });
  cout << endl;

  cout << "DFS:" << endl;
  parse_graph->visit_dfs(0, [](ParseGraph &pg, int n){
      //cout << pg.name(n) << " - " << pg.substr(n) << endl;
    });
  cout << endl;



  for (auto s : statements) {
    cout << "LEAF:" << s << endl;
    parse_graph->visit_bottom_up(s, [](ParseGraph &pg, int n){
        if (pg.name(n) == "stat")
          cout << "stat name:" << pg.substr(pg.children(n)[0]) << endl;
      cout << n << " {" << pg.name(n) << " : " << pg.substr(n) << endl;
      //for (int c : pg.nodes[n].children) {
      //  cout << c << " }" << pg.name(c) << " : " << pg.substr(c) << endl;
      //  if (bla[c] == 1)
      //    throw "";
      //}
    });
  }
  
}
