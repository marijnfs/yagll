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
  int cursor, rule;
  bool operator<(NodeIndex const &other) const {
    if (cursor != other.cursor)
      return cursor < other.cursor;
    return rule < other.rule;
  }

  bool operator>(NodeIndex const &other) const {
    if (cursor != other.cursor)
      return cursor > other.cursor;
    return rule > other.rule;
  }
  
};


struct Head {
  NodeIndex nodeindex;
  int index;

  bool operator<(Head &other) {
    return nodeindex < other.nodeindex;
  }
  
  bool operator>(Head &other) {
    return nodeindex > other.nodeindex;
  }
};

typedef priority_queue<Head, vector<Head>, greater<Head> > NodeQueue;

struct Graph {
  vector<int> cursors, rules;
  vector<vector<int>> calls, returns, prevs;
  vector<int> property_parents;
  int n_nodes;
  
  map<NodeIndex, int> index_map;

  bool exists(NodeIndex index) {
    return index_map.count(index) > 0;
  }

  int operator()(NodeIndex index)  {
    return index_map[index];
  }

  int property_parent(int n) {
    int p(n);
    while (property_parents[p] != p)
      p = property_parents[p];
    property_parents[n] = p; //compression
    return p;
  }
  
  int new_node(int cursor, int rule) {
    int new_index = n_nodes;
    cursors.push_back(cursor);
    rules.push_back(rule);

    calls.push_back(vector<int>());
    returns.push_back(vector<int>());
    prevs.push_back(vector<int>());

    index_map[NodeIndex{cursor, rule}] = new_index;
    ++n_nodes;
    return new_index;
  }
  
  int new_child_node(int parent, int rule) {
    int new_index = new_node(cursors[parent], rule);
    calls[new_index].push_back(parent);
    property_parents[new_index] = new_index; //child owns properties
    prevs[new_index].push_back(parent);
    return new_index;
  }

  int new_next_node(int brother, int n_matched) {
    int new_index = new_node(cursors[brother] + n_matched, rules[brother] + 1);
    property_parents[new_index] = brother;
    prevs[new_index].push_back(brother);
    return new_index;
  }

  vector<int> &return_node(int from) {
    int prop_parent = property_parent(from);
    for (int call : calls[prop_parent]) {
      returns[call].push_back(from);
      prevs[call].push_back(from);
    }
    return calls[prop_parent];
  }
  
};

struct Op {
  string name;
  
Op(string name_):name(name_){}
  virtual int operator()(string &buffer, int head, Graph &graph);
};

struct MatchOp : public Op {
  RE2 reg;
  string match_string;
  
 MatchOp(string name, string match): Op(name), reg(match), match_string(match){}
  
  virtual int operator()(string &buffer, int head, Graph &graph, NodeQueue &queue) {
    int cursor = graph.cursors[head];
    re2::StringPiece match;
    if (reg.Match(buffer, cursor, buffer.size(), RE2::ANCHOR_START, &match, 1)) {
      cout << "Matched " << match.length() << endl;
    }
    return 0;
  }
  
};

struct SpawnOp : public Op {
  vector<string> spawn_rule_names;
  vector<int> spawn_rules;
  
  virtual int operator()(string &buffer, int head, Graph &graph, NodeQueue &queue);
};

struct EndOp : public Op {
  virtual int operator()(string &buffer, int head, Graph &graph, NodeQueue &queue);
};

struct ReturnOp : public Op {
  virtual int operator()(string &buffer, int head, Graph &graph, NodeQueue &queue);
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
