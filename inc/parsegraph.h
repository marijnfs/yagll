#ifndef __PARSE_GRAPH_H__
#define __PARSE_GRAPH_H__

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <sstream>

struct ParsedNode {
  int n = -1;
  ParsedNode(){}
  ParsedNode(int n_) :n(n_) {}
  std::vector<int> parents, children;
};

struct ParseGraph;
struct SearchNode {
  int N = -1;
  ParseGraph *pg = 0;

  std::string type();

  std::string text();

  template <typename T>
  T text_to() {
    auto text_str = text();
    std::istringstream iss(text_str);
    T val;
    iss >> val;
    return val;
  }


  SearchNode child(int n = 0);

  SearchNode child(std::string type);

  std::vector<SearchNode> children();

  std::vector<SearchNode> get_all(std::string type);
  
  std::vector<SearchNode> int_to_searchnodes(std::vector<int> &ints);

  //visit_bottom_up

  //visit_dfs_filtered, 

  bool valid() { return N != -1; }
};

struct GraphCallback {
  virtual bool match(int n) {
    return true;
  }

  virtual bool add_children(int n) {
    return true;
  }

  virtual void operator()(int n) {
  }
};


struct TypeGraphCallback : public GraphCallback {
  typedef std::function<void(int)> CallbackFunc;
  std::map<std::string, CallbackFunc> callbacks;
  std::set<std::string> types_set;
  
  ParseGraph *pg = 0;

  TypeGraphCallback(ParseGraph *pg_);

  void register_callback(std::string type, CallbackFunc func);

  virtual void operator()(int n) override;

  virtual bool match(int n) override;

  virtual bool add_children(int n) override;
};

struct BottomUpGraphCallback : public TypeGraphCallback {
  virtual void run_default(int n);
  virtual void operator()(int n) override;

  virtual bool match(int n) override;

  virtual bool add_children(int n) override;
};

struct ParseGraph {
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

  void pprint(std::ostream &out, int n = 0, int depth = 0);

  bool has_type(int n);

  std::string text(int n);

  std::string const &type(int n);

  std::vector<int> &children(int n) { return nodes[n].children; }
  std::vector<int> &parents(int n) { return nodes[n].parents; }
  
  void bottom_up(GraphCallback &callback, int root = 0);

  void top_down(GraphCallback &callback, int root = 0);
  
  void sort_children(std::function<bool(int a, int b)> cmp);

  int size() { return nodes.size(); }

  SearchNode operator()(int n);

  SearchNode root();
};


#endif
