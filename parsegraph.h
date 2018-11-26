#ifndef __PARSE_GRAPH_H__
#define __PARSE_GRAPH_H__

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

struct ParsedNode {
  int n = -1;
  ParsedNode(){}
  ParsedNode(int n_) :n(n_) {}
  std::vector<int> parents, children;
};

struct ParseGraph {
  std::vector<ParsedNode> nodes;

  std::vector<int> starts;
  std::vector<int> ends;
  std::vector<int> name_ids;
  //std::vector<int> levels;
  std::vector<bool> cleanup; // boolean indicating whether a node is used,
                             // relevant for compacting

  std::map<int, std::string> name_map;
  std::map<std::string, int> rname_map;

  std::string buffer;

  /// compact runs through nodes and removes the ones that are marked for
  /// cleanup adjusts all relevant indices as required, also in parsed nodes
  void compact();

  void add_node(int nodeid, int start, int end, std::string name);

  void add_connection(int p, int c);
  
  int add_rulename(std::string name);
  
  void filter(std::function<void(ParseGraph &, int)> callback);

  int get_one(int root, std::string name);

  std::vector<int> get_all(int root, std::string name);

  std::vector<int> get_all_recursive(int root, std::string name);

  void print_dot(std::string filename);

  bool has_name(int n);

  std::string substr(int n);

  std::string const &name(int n);

  int root();

  typedef std::function<void(ParseGraph &pg, int n)> Callback;

  std::vector<int> &children(int n) { return nodes[n].children; }
  std::vector<int> &parents(int n) { return nodes[n].parents; }
  
  // Visit breadth-first search
  void visit_bfs(int root, Callback cb);

  // Visit depth-first search
  void visit_dfs(int root, Callback cb);
  
  // Visit bottom up, starting from leafs
  // assuring when a node is visited, all its leafs have already visited
  void visit_bottom_up(int root, Callback cb);

  int size() { return nodes.size(); }
};

#endif
