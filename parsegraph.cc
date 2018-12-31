#include "parsegraph.h"
#include "const.h"
#include "stringexception.h"

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


vector<int> ParseGraph::get_all(int root, string filter_name) {
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

vector<int> ParseGraph::get_all_recursive(int root, string filter_name) {
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
    }
    
    for (int c : nodes[n].children)
      s.push(c);
  }

  sort(result.begin(), result.end(), [this](int n1, int n2) { return starts[n1] < starts[n2]; });
  return result;
}

int ParseGraph::get_one(int root, string search_name) {
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

string const &ParseGraph::name(int n) {
  if (!has_name(n)) {
    static string empty("");
    return empty;
  }
  return name_map[name_ids[n]];
}

string ParseGraph::substr(int n) {
  return buffer.substr(starts[n], ends[n] - starts[n]);
}

void ParseGraph::filter(function<void(ParseGraph &, int)> callback) {
  fill(cleanup.begin(), cleanup.end(), false);
  for (int i(0); i < nodes.size(); ++i)
    callback(*this, i);
  compact_cleanup();
}

void ParseGraph::squeeze(function<bool(ParseGraph &, int)> callback) {
  fill(cleanup.begin(), cleanup.end(), false);

  vector<bool> visited(size());
  
  queue<int> q;
  q.push(0); //start at 0
  
  while (q.size()) {
    int n = q.front();
    q.pop();

    if (visited[n])
      continue;
    visited[n] = true;

    auto sq = callback(*this, n);
    if (sq)
      for (int c : nodes[n].children)
        cleanup[c] = true; //overassign cleanup to kids

    if (cleanup[n] && !sq)
      cleanup[n] = false; //fix 'erronously set' cleanup
    
    for (int c : nodes[n].children)
      q.push(c);
  }

  compact_cleanup();
}

void ParseGraph::remove(function<bool(ParseGraph &, int)> callback) {
  fill(cleanup.begin(), cleanup.end(), false);
  vector<bool> visited(size());
  
  queue<int> q;
  q.push(0); //start at 0
  
  while (q.size()) {
    int n = q.front();
    q.pop();

    if (visited[n])
      continue;
    visited[n] = true;

    auto rem = cleanup[n] || callback(*this, n);
    cleanup[n] = rem;

    for (int c : nodes[n].children) {
      if (rem) 
        cleanup[c] = true;      
      q.push(c);
    }
  }

  remove_cleanup();
}

void ParseGraph::compact_cleanup() {
  // Make sure all removed nodes pass over their children and parent links
  for (int n(0); n < nodes.size(); ++n)
    if (cleanup[n]) {
      for (auto p : nodes[n].parents)
        copy(nodes[n].children.begin(),
             nodes[n].children.end(),
             back_inserter(nodes[p].children));
             
      for (auto c : nodes[n].children)
        copy(nodes[n].parents.begin(),
             nodes[n].parents.end(),
             back_inserter(nodes[c].parents));
    }

  remove_cleanup();
}

void ParseGraph::remove_cleanup() {
  // First create a node map, -1 for removing nodes
  int N(0);
  vector<int> node_map(nodes.size());
  for (int n(0); n < nodes.size(); ++n)
    if (cleanup[n])
      node_map[n] = -1;
    else
      node_map[n] = N++;

  // Map over all indices, also for parents and children in all nodes
  for (int n(0); n < nodes.size(); ++n) {
    if (!cleanup[n]) {
      vector<int> new_parents, new_children;
      for (auto p : nodes[n].parents)
        if (!cleanup[p])
          new_parents.push_back(node_map[p]);
      for (auto c : nodes[n].children)
        if (!cleanup[c])
          new_children.push_back(node_map[c]);
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
}

// Visit breadth-first search
void ParseGraph::visit_bfs(int root, Callback cb) {
  vector<bool> visited(size());
  
  queue<int> q;
  q.push(root); //start at 0
  
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
void ParseGraph::visit_dfs(int root, Callback cb) {
  vector<bool> visited(size());
  
  stack<int> q;
  q.push(root); 
  
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
void ParseGraph::visit_bottom_up(int root, Callback cb) {
  vector<int> ordered_n;
  ordered_n.reserve(nodes.size());
  visit_dfs(root, [&ordered_n](ParseGraph &pg, int n) {
      ordered_n.push_back(n);
    });
  reverse(ordered_n.begin(), ordered_n.end());
  
  for (int n : ordered_n)
    cb(*this, n);
}


void ParseGraph::add_node(int nodeid, int start, int end, string name) {
  nodes.push_back(ParsedNode(nodeid));
  starts.push_back(start);
  ends.push_back(end);
  cleanup.push_back(false);

  int name_id = -1;
  if (name.size())
    name_id = add_rulename(name);
  name_ids.push_back(name_id);
}

void ParseGraph::add_connection(int p, int c) {
  if (p >= nodes.size() || c >= nodes.size()) {
    cerr << "OUTSIDE CONNECTION" << endl;
    return;
  }

  nodes[p].children.push_back(c);
  nodes[c].parents.push_back(p);
}

int ParseGraph::add_rulename(string name) {
  int name_id(-1);
  if (!rname_map.count(name)) {
    name_id = rname_map.size();
    rname_map[name] = name_id;
    name_map[name_id] = name;
  } else
    name_id = rname_map[name];
  return name_id;
}
