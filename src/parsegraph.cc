#include "parsegraph.h"
#include "yagll.h"
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
    string type = type_map.count(type_ids[n]) ? type_map[type_ids[n]] : "";
    string sub =
        nodes[n].children.size() == 0
            ? string("'") + buffer.substr(starts[n], ends[n] - starts[n]) + "'"
            : "";
    replace(sub.begin(), sub.end(), '"', '#');
    dotfile << "node" << n << " " << " [label=\"" << type << " " << sub << " "
            << starts[n] << ":" << ends[n] << "\"]" << endl;
    for (auto p : nodes[n].parents)
      dotfile << "node" << n << " -> "
              << "node" << p << endl;
  }
  dotfile << "}" << endl;
}

void ParseGraph::pprint(ostream &out, int n, std::vector<uint8_t> depths) {
  for (auto &d : depths)
    if (&d == &depths.back() && !d)
      cout << "+";
    else if (d)
      cout << "|";
    else
      cout << " ";
  if (children(n).empty())
    cout << type(n) << "(" << text(n) << ")" << endl;
  else
    cout << type(n) << endl;
  
  depths.emplace_back(false);
  
  auto children_ = children(n);
  for (auto &child : children_) {
    depths.back() = child != children_.back();
    pprint(out, child, depths);
  }
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

    if (type(n) == filter_name) {
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

    if (type(n) == filter_name) {
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
      // cout << c << " |" << type(c) << "|" << endl;
      if (type(c) == search_name)
        return c;
      else
        s.push(c);
    }
  }

  return -1;
}

int ParseGraph::get_one(int root, set<string> search_names) {
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
      // cout << c << " |" << type(c) << "|" << endl;
      if (search_names.count(type(c)))
        return c;
      else
        s.push(c);
    }
  }

  return -1;
}

bool ParseGraph::has_type(int n) { return type_ids[n] != -1; }

string const &ParseGraph::type(int n) {
  if (!has_type(n)) {
    static string empty("");
    return empty;
  }
  return type_map[type_ids[n]];
}

string ParseGraph::text(int n) {
  return buffer.substr(starts[n], ends[n] - starts[n]);
}

void ParseGraph::filter(BoolCallback callback) {
  fill(cleanup.begin(), cleanup.end(), false);
  for (int i(0); i < nodes.size(); ++i)
    cleanup[i] = callback(*this, i);
  compact_cleanup();
}

void ParseGraph::squeeze(BoolCallback callback) {
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

void ParseGraph::remove(BoolCallback callback) {
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
      type_ids[node_map[n]] = type_ids[n];
    }
  }

  // Resize everything
  nodes.resize(N);
  starts.resize(N);
  ends.resize(N);
  type_ids.resize(N);
  cleanup.resize(N);
}

void ParseGraph::bottom_up(GraphCallback &callback, int root) {
  vector<bool> visited(size());
  vector<int> matched;
  matched.reserve(size()); //reserve to prevent many reallocs, could be optimized by allocating it once and reusing it
  
  stack<int> q;
  q.push(root);
  
  while (q.size()) {
    int n = q.top();
    q.pop();

    if (visited[n])
      continue;
    visited[n] = true;

    if (callback.match(n))
      matched.push_back(n);

    //push in reverse order on the stack, since last pushed will be popped first
    if (callback.add_children(n))
      for (auto it = children(n).rbegin(); it != children(n).rend(); ++it)
        q.push(*it);
  }
  
  auto it_end = matched.rend();
  for (auto it_n = matched.rbegin(); it_n != it_end; ++it_n)
    callback(*it_n);
}

void ParseGraph::top_down(GraphCallback &callback, int root) {
  cout << "tpo down" << endl;
  vector<bool> visited(size());
  
  stack<int> q;
  q.push(root);
  
  while (q.size()) {
    int n = q.top();
    cout << n << endl;
    q.pop();

    if (visited[n])
      continue;
    visited[n] = true;

    if (callback.match(n))
      callback(n);

    //push in reverse order on the stack, since last pushed will be popped first
    if (callback.add_children(n))
      for (auto it = children(n).rbegin(); it != children(n).rend(); ++it)
        q.push(*it);
  }  
}


void ParseGraph::sort_children(function<bool(int a, int b)> cmp) {
  vector<bool> visited(size());
  
  queue<int> q;
  q.push(0); //start at 0
  
  while (q.size()) {
    int n = q.front();
    q.pop();

    if (visited[n])
      continue;
    visited[n] = true;

    auto &children = nodes[n].children;

    sort(children.begin(), children.end(), cmp);
    for (int c : nodes[n].children)
      q.push(c);
  }
}

void ParseGraph::visit(GraphCallback &callback, int root) {
  if (callback.callback_mode() == GraphCallback::TOP_DOWN)
    top_down(callback, root);
  else if (callback.callback_mode() == GraphCallback::BOTTOM_UP)
    bottom_up(callback, root);
  else
    throw std::runtime_error("callback mode not set");
}

void ParseGraph::add_node(int nodeid, int start, int end, string type) {
  nodes.push_back(ParsedNode(nodeid));
  starts.push_back(start);
  ends.push_back(end);
  cleanup.push_back(false);

  int type_id = -1;
  if (type.size())
    type_id = add_ruletype(type);
  type_ids.push_back(type_id);
}

void ParseGraph::add_connection(int p, int c) {
  if (p >= nodes.size() || c >= nodes.size()) {
    cerr << "OUTSIDE CONNECTION" << endl;
    return;
  }

  nodes[p].children.push_back(c);
  nodes[c].parents.push_back(p);
}

int ParseGraph::add_ruletype(string type) {
  int type_id(-1);
  if (!reverse_type_map.count(type)) {
    type_id = reverse_type_map.size();
    reverse_type_map[type] = type_id;
    type_map[type_id] = type;
  } else
    type_id = reverse_type_map[type];
  return type_id;
}

SearchNode ParseGraph::operator()(int n) {
  return SearchNode{n, this};
}

SearchNode ParseGraph::root() {
  return SearchNode{0, this};
}

void tree_print(ParseGraph &pg, int n, int depth) {
  for (int i(0); i < depth; ++i)
    cout << "  ";
  if (pg.type(n) == "name" || pg.type(n) == "varname" || pg.type(n) == "value" || pg.type(n) == "number")
    cout << "[" << pg.starts[n] << "] " << pg.type(n) << "(" << pg.text(n) << ")" << endl;
  else
    cout << pg.type(n) << endl;

  for (auto c : pg.children(n))
    tree_print(pg, c, depth + 1); 
}
