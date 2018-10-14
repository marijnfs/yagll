#ifndef __PARSE_GRAPH_H__
#define __PARSE_GRAPH_H__

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

struct ParsedNode {
  int n = -1;

  std::set<int> parents, children;
};

struct ParseGraph {
  std::vector<ParsedNode> nodes;

  std::vector<int> starts;
  std::vector<int> ends;
  std::vector<int> name_ids;
  std::vector<int> levels;
  std::vector<bool> cleanup; // boolean indicating whether a node is used,
                             // relevant for compacting

  std::map<int, std::string> name_map;
  std::map<std::string, int> rname_map;

  std::string buffer;

  /// compact runs through nodes and removes the ones that are marked for
  /// cleanup adjusts all relevant indices as required, also in parsed nodes
  void compact();

  void filter(std::function<void(ParseGraph &, int)> callback);

  // get nodes with certain name, connected to given root node, allow
  // connections through third param
  std::vector<int> get_connected(int root, std::string filter_name,
                                 std::string connection);

  std::vector<int> get_connected(int root, std::string filter_name);

  int get_one(int root, std::string name);

  void print_dot(std::string filename);

  bool has_name(int n);

  std::string substr(int n);

  std::string name(int n);

  int root();

  typedef std::function<void(ParseGraph &pg, int n)> Callback;

  // Visit breadth-first search
  void visit_bfs(Callback cb);

  // Visit depth-first search
  void visit_dfs(Callback cb);
  
  // Visit bottom up, starting from leafs
  // assuring when a node is visited, all its leafs have already visited
  void visit_bottom_up(Callback cb);

  int size() { return nodes.size(); }
};

#endif
