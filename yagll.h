#ifndef __PARSER_H__
#define __PARSER_H__

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "inc/parsegraph.h"
#include "inc/ruleset.h"

#include "inc/stringexception.h"

struct NodeIndex {
  int cursor = -1, rule = -1;
  int id = -1;
  int prev = -1;
  
  bool operator<(NodeIndex const &other) const;
  bool operator>(NodeIndex const &other) const;
};

std::ostream &operator<<(std::ostream &out, NodeIndex &ni);

typedef std::priority_queue<NodeIndex, std::vector<NodeIndex>, std::less<NodeIndex>> node_queue;
struct Parser {
  RuleSet ruleset;

  // Parser keeps the whole parsing state here

  std::vector<NodeIndex> nodes; // all nodes are stored here
  std::vector<int> properties;  // properties index, pointing of node containing
                                // shared info of nodes, such as:
  std::vector<std::set<int>>
      parents; // who is your parent? Can be multiple nodes that happen to spawn
               // this rule at this cursor
  std::vector<std::set<int>> ends; // where does this rule end, info needed when
                                   // spawning already ended nodes
  std::vector<std::set<int>> crumbs; // crumbs, for de-parsing
  std::vector<int> prevs;
  
  std::set<NodeIndex>
      node_occurence; // occurence set, checking if a node already exists
  
  node_queue heads; // active part, a queue with sorted
                                        // nodes

  std::string buffer;
  std::string input_filename;
  
  int end_node = 0;
  int furthest = 0; // aux var, to see how far we got in case of fail

  Parser(std::istream &infile, LoadType load_type = LOAD_YOPLYOPL);

  // add a node
  // does not check whether it exists
  void push_node(int cursor, int rule, int prop_node, int parent = -1,
                 int crumb = -1, int prev = -1);

  // parse a file
  std::unique_ptr<ParseGraph> parse(std::istream &input_file);

  void load(std::istream &infile);

  void process();

  std::unique_ptr<ParseGraph> post_process();

  void dot_graph_debug(std::string filename);

  void fail_message();

  //resets parser state, but not ruleset. To be used to before parsing another buffer. 
  void reset();

  int size() { return nodes.size(); }
};

struct ParseGraph;
struct SearchNode {
  int N = -1;
  ParseGraph *pg = 0;

  std::string type();

  std::string text();

  template <typename T>
  T text_to() {
    auto text_str = text();
    std::istringstream iss(text_str);
    T val;
    iss >> val;
    return val;
  }


  SearchNode child(int n = 0);

  SearchNode child(std::string type);

  std::vector<SearchNode> children();

  std::vector<SearchNode> get_all(std::string type);
  
  std::vector<SearchNode> int_to_searchnodes(std::vector<int> &ints);

  void visit(GraphCallback &callback);

  void bottom_up(GraphCallback &callback);

  void top_down(GraphCallback &callback);

  bool valid() { return N != -1; }
};

void tree_print(ParseGraph &pg, int n, int depth);

template <typename T> inline T &last(std::vector<T> &v) {
  return v[v.size() - 1];
}

#endif
