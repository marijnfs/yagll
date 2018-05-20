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
  std::vector<bool> cleanup; // boolean indicating whether a node is used,
                             // relevant for compacting

  std::map<int, std::string> name_map;
  std::map<std::string, int> rname_map;

  std::string buffer;

  /// compact runs through nodes and removes the ones that are marked for
  /// cleanup adjusts all relevant indices as required, also in parsed nodes
  void compact();

  void filter(std::function<void(ParseGraph &, int)> callback);

  void print_dot(std::string filename);

  int size() { return nodes.size(); }
};

#endif
