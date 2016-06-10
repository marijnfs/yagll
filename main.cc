#include "graph.h"

#include <iostream>

using namespace std;


int main(int argc, char **argv) {  
  RuleSet rule_set;
  rule_set.read_from_file("grammar.gram");
  
  Graph graph;
  Parser parser{rule_set, graph};
  parser.parse_file("test.txt");
  
  return 0;
}
