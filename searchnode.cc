#include "parsegraph.h"

#include <iostream>

using namespace std;

std::string SearchNode::text() {
  return pg->text(N);
}

SearchNode SearchNode::child(int n) {
  return SearchNode{pg->children(N)[n], pg};
}

SearchNode SearchNode::child(string type) {
  auto &children = pg->children(N);
  for (int child : children)
    if (pg->type(child) == type)
      return SearchNode{child, pg};

  throw std::runtime_error("Couldn't find child type");
}

std::vector<SearchNode> SearchNode::children() {
  auto &children_ids = pg->children(N);
  return int_to_searchnodes(children_ids);
}

std::vector<SearchNode> SearchNode::get_all(std::string type) {
  auto matches = pg->get_all(N, type);
  return int_to_searchnodes(matches);
}
  
  

std::vector<SearchNode> SearchNode::int_to_searchnodes(std::vector<int> &ints) {
  std::vector<SearchNode> nodes(ints.size());

  auto it = ints.begin();
  auto const it_end = ints.end();
  auto it_vec = nodes.begin();
  for (; it != it_end; ++it, ++it_vec)
    *it_vec = SearchNode{*it, pg};
  return nodes;
}
