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

#include "parsegraph.h"
#include "ruleset.h"

struct NodeIndex {
  int cursor = -1, rule = -1;
  int id = -1;
  int prev = -1;
  
  bool operator<(NodeIndex const &other) const;
  bool operator>(NodeIndex const &other) const;
};

std::ostream &operator<<(std::ostream &out, NodeIndex &ni);

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
  
  std::priority_queue<NodeIndex> heads; // active part, a queue with sorted
                                        // nodes

  std::string buffer;
  std::string input_filename;
  
  int end_node = 0;
  int furthest = 0; // aux var, to see how far we got in case of fail

  Parser(std::string gram_file, LoadType load_type = LOAD_YOPLYOPL);

  // add a node
  // does not check whether it exists
  void push_node(int cursor, int rule, int prop_node, int parent = -1,
                 int crumb = -1, int prev = -1);

  // parse a file
  std::unique_ptr<ParseGraph> parse(std::string input_file);

  void load(std::string filename);

  void process();

  std::unique_ptr<ParseGraph> post_process();

  void dot_graph_debug(std::string filename);

  void fail_message();

  //resets parser state, but not ruleset. To be used to before parsing another buffer. 
  void reset();

  int size() { return nodes.size(); }
};

template <typename T> inline T &last(std::vector<T> &v) {
  return v[v.size() - 1];
}

#endif
