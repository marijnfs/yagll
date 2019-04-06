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
  typedef std::function<void(ParseGraph &pg, int n)> Callback;
  typedef std::function<bool(ParseGraph &pg, int n)> BoolCallback;

  std::vector<ParsedNode> nodes;

  std::vector<int> starts;
  std::vector<int> ends;
  std::vector<int> type_ids;
  //std::vector<int> levels;
  std::vector<bool> cleanup; // boolean indicating whether a node is used,
                             // relevant for compacting

  std::map<int, std::string> type_map;
  std::map<std::string, int> reverse_type_map;

  std::string buffer;


  void add_node(int nodeid, int start, int end, std::string type);

  void add_connection(int p, int c);
  
  int add_ruletype(std::string type);
  
  void filter(BoolCallback callback);

  void squeeze(BoolCallback callback);
  
  void remove(BoolCallback callback);

  /// compact runs through nodes and removes the ones that are marked for
  /// cleanup adjusts all relevant indices as required, also in parsed nodes
  void compact_cleanup();

  void remove_cleanup();

  int get_one(int root, std::string type);

  int get_one(int root, std::set<std::string> search_names);

  std::vector<int> get_all(int root, std::string type);

  std::vector<int> get_all_recursive(int root, std::string type);

  void print_dot(std::string filename);

  bool has_type(int n);

  std::string text(int n);

  std::string const &type(int n);

  int root();


  std::vector<int> &children(int n) { return nodes[n].children; }
  std::vector<int> &parents(int n) { return nodes[n].parents; }
  
  // Visit breadth-first search
  void visit_bfs(int root, Callback cb);

  // Visit depth-first search
  void visit_dfs(int root, Callback cb);

  // Visit breadth-first search, filtered
  void visit_bfs_filtered(int root, BoolCallback cb);

  // Visit depth-first search, filtered
  void visit_dfs_filtered(int root, BoolCallback cb);
  
  // Visit bottom up, shttp://vps66856.doeigeld.comtarting from leafs
  // assuring when a node is visited, all its leafs have already visited
  void visit_bottom_up(int root, Callback cb);

  void visit_bottom_up_filtered(int root, Callback cb, BoolCallback filter);

  void sort_children(std::function<bool(int a, int b)> cmp);

  int size() { return nodes.size(); }
};

#endif
