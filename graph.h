#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <map>
#include <string>
#include <set>
#include <queue>
#include <cassert>
#include <regex>
#include <re2/re2.h>
using namespace std;

struct NodeIndex {
  int rule, index;
  bool operator<(NodeIndex const &other) const {
    if (index != other.index)
      return index < other.index;
    return rule < other.rule;
  }

  bool operator>(NodeIndex const &other) const {
    if (index != other.index)
      return index > other.index;
    return rule > other.rule;
  }
  
};


struct Node {
  NodeIndex nodeindex;
  vector<int> parents, previous;

  bool operator<(Node &other) {
    return nodeindex < other.nodeindex;
  }
  
  bool operator>(Node &other) {
    return nodeindex > other.nodeindex;
  }
};

typedef priority_queue<Node, vector<Node>, greater<Node> > NodeQueue;

struct Graph {
  vector<Node*> nodes;
  map<NodeIndex, int> index_map;

  Node &operator()(NodeIndex index) {
    return *nodes[index_map[index]];
  }
  
  Node &operator[](int index) {
    return *nodes[index];
  }

  bool exists(NodeIndex index) {
    return index_map.count(index) > 0;
  }
  
};

struct Op {
  string name;
  
Op(string name_):name(name_){}
  virtual int operator()(Node &node, Graph &graph);
};

struct MatchOp : public Op {
  string match_command;
  
  virtual int operator()(Node &node, Graph &graph);
};

struct SpawnOp : public Op {
  vector<int> spawn_rules;
  virtual int operator()(Node &node, Graph &graph);
};

struct EndOp : public Op {
  virtual int operator()(Node &node, Graph &graph);
};


struct RuleSet {
  vector<Op*> operations;

  map<string, int> name_map;

  void add_rule(){}
  void read_from_file(string path){}
};

string read_all_file(string path) {
  ifstream file(path);
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string buffer(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size);
    return buffer;
}

struct Parser {
  RuleSet ruleset;
  Graph graph;

  void reset_graph();
  
  void parse_file(string name) {

    
  }
};


#endif
