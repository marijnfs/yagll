#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <vector>

struct NodeIndex {
  int rule, index;
};


struct Node {
  NodeIndex nodeindex;
  vector<int> parents, previous;
};

struct Graph {
  vector<Node*> nodes;
  map<NodeIndex, int> index_map;

  Node &operator()(NodeIndex index) {
    return nodes[index_map[index]];
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

  void add_rule();
  void read_from_file();
};

struct Parser {
  RuleSet ruleset;
  Graph graph;

  void reset_graph();
  
  void run() {
    
  }
};


#endif
