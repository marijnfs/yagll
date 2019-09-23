#include "yagll.h"

#include <iostream>

using namespace std;

std::string SearchNode::text() {
  return pg->text(N);
}

std::string SearchNode::type() {
  return pg->type(N);
}

SearchNode SearchNode::child(int n) {
  return SearchNode{pg->children(N)[n], pg};
}

SearchNode SearchNode::child(string type) {
  auto &children = pg->children(N);
  for (int child : children)
    if (pg->type(child) == type)
      return SearchNode{child, pg};

  return SearchNode{-1, pg};
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

void SearchNode::bottom_up(GraphCallback &callback) {
  pg->bottom_up(callback, N);
}

void SearchNode::top_down(GraphCallback &callback) {
  pg->top_down(callback, N);
}

void SearchNode::visit(GraphCallback &callback) {
  pg->visit(callback, N);
}
