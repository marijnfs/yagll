#include "parsegraph.h"
#include "const.h"

#include <algorithm>
#include <fstream>

#include <queue>
#include <stack>

#include <iostream>

using namespace std;

void ParseGraph::print_dot(string filename) {
  ofstream dotfile(filename);

  dotfile << "digraph parsetree {" << endl;
  for (int n(0); n < nodes.size(); ++n) {
    string name = name_map.count(name_ids[n]) ? name_map[name_ids[n]] : "";
    string sub =
        nodes[n].children.size() == 0
            ? string("'") + buffer.substr(starts[n], ends[n] - starts[n]) + "'"
            : "";
    replace(sub.begin(), sub.end(), '"', '#');
    dotfile << "node" << n << " " << " [label=\"" << name << " " << sub << " "
            << starts[n] << ":" << ends[n] << "\"]" << endl;
    for (auto p : nodes[n].parents)
      dotfile << "node" << n << " -> "
              << "node" << p << endl;
  }
  dotfile << "}" << endl;
}

int ParseGraph::root() {
  for (int n(0); n < nodes.size(); ++n)
    if (name(n) == "ROOT")
      return n;
  return -1;
}

std::vector<int> ParseGraph::get_connected(int root, std::string filter_name,
                                           std::string connection) {
  throw "";
  if (root < 0)
    throw StringException("can't get connected nodes, provided root is negative");
  vector<int> result;
  stack<int> s;
  s.push(root);

  set<int> visited;
  while (s.size()) {
    int n = s.top();
    s.pop();

    // prevent loops
    if (visited.count(n))
      continue;
    visited.insert(n);

    for (int c : nodes[n].children) {
      // cout << c << " |" << name(c) << "|" << endl;
      if (name(c) == filter_name)
        result.push_back(c);
      if (name(c) == connection)
        s.push(c);
    }
  }

  return result;
}

std::vector<int> ParseGraph::get_connected(int root, std::string filter_name) {
  //cout << "get connected for: " << root << " curs:" << starts[root] << " " << name(root) << " looking for: " << filter_name << endl;
  vector<int> result;
  queue<int> s;
  s.push(root);

  set<int> visited;
  while (s.size()) {
    int n = s.front();
    s.pop();

    // prevent loops
    if (visited.count(n))
      continue;
    visited.insert(n);

    if (name(n) == filter_name) {
      result.push_back(n);
      continue;
    }
    
    for (int c : nodes[n].children)
      s.push(c);
  }

  sort(result.begin(), result.end(), [this](int n1, int n2) { return starts[n1] < starts[n2]; });
  return result;
}

int ParseGraph::get_one(int root, std::string search_name) {
  if (root < 0)
    throw StringException("get_one called on neg root");
  queue<int> s;
  s.push(root);

  set<int> visited;
  while (s.size()) {
    int n = s.front();
    s.pop();

    // prevent loops
    if (visited.count(n))
      continue;
    visited.insert(n);

    for (int c : nodes[n].children) {
      // cout << c << " |" << name(c) << "|" << endl;
      if (name(c) == search_name)
        return c;
      else
        s.push(c);
    }
  }

  return -1;
}

bool ParseGraph::has_name(int n) { return name_ids[n] != -1; }

string ParseGraph::name(int n) {
  if (!has_name(n))
    return "";
  return name_map[name_ids[n]];
}

string ParseGraph::substr(int n) {
  return buffer.substr(starts[n], ends[n] - starts[n]);
}

void ParseGraph::filter(function<void(ParseGraph &, int)> callback) {
  fill(cleanup.begin(), cleanup.end(), false);
  for (int i(0); i < nodes.size(); ++i)
    callback(*this, i);
  compact();
}

void ParseGraph::compact() {
  // First create a node map, -1 for removing nodes
  int N(0);
  vector<int> node_map(nodes.size());
  for (int n(0); n < nodes.size(); ++n)
    if (cleanup[n])
      node_map[n] = -1;
    else
      node_map[n] = N++;

  // Make sure all removed nodes pass over their children and parent links
  for (int n(0); n < nodes.size(); ++n)
    if (cleanup[n]) {
      for (auto p : nodes[n].parents)
        nodes[p].children.insert(nodes[n].children.begin(),
                                 nodes[n].children.end());
      for (auto c : nodes[n].children)
        nodes[c].parents.insert(nodes[n].parents.begin(),
                                nodes[n].parents.end());
    }

  // Map over all indices, also for parents and children in all nodes
  for (int n(0); n < nodes.size(); ++n) {
    if (!cleanup[n]) {
      set<int> new_parents, new_children;
      for (auto p : nodes[n].parents)
        if (!cleanup[p])
          new_parents.insert(node_map[p]);
      for (auto c : nodes[n].children)
        if (!cleanup[c])
          new_children.insert(node_map[c]);
      nodes[n].parents = new_parents;
      nodes[n].children = new_children;

      nodes[node_map[n]] = nodes[n];
      starts[node_map[n]] = starts[n];
      ends[node_map[n]] = ends[n];
      name_ids[node_map[n]] = name_ids[n];
    }
  }

  // Resize everything
  nodes.resize(N);
  starts.resize(N);
  ends.resize(N);
  name_ids.resize(N);
  cleanup.resize(N);
  fill(cleanup.begin(), cleanup.end(), false);
}

// Visit breadth-first search
void ParseGraph::visit_bfs(Callback cb) {
  vector<bool> visited(size());
  
  queue<int> q;
  q.push(0); //start at 0
  
  while (q.size()) {
    int n = q.front();
    q.pop();

    if (visited[n])
      continue;
    visited[n] = true;

    cb(*this, n);

    for (int c : nodes[n].children)
      q.push(c);
    
  }
}

// Visit depth-first search
void ParseGraph::visit_dfs(Callback cb) {
  vector<bool> visited(size());
  
  stack<int> q;
  q.push(0); //start at 0
  
  while (q.size()) {
    int n = q.top();
    q.pop();

    if (visited[n])
      continue;
    visited[n] = true;

    cb(*this, n);

    for (int c : nodes[n].children)
      q.push(c);
    
  }
}

// Visit bottom up, starting from leafs
// assuring when a node is visited, all its leafs have already visited
void ParseGraph::visit_bottom_up(Callback cb) {
  vector<int> ordered_n;
  ordered_n.reserve(nodes.size());
  visit_bfs([&ordered_n](ParseGraph &pg, int n) {
      ordered_n.push_back(n);
    });
  reverse(ordered_n.begin(), ordered_n.end());
  
  for (int n : ordered_n)
    cb(*this, n);
}
