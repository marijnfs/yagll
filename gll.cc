#include <iostream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <map>
#include <string>
#include <set>
#include <queue>
#include <cassert>

using namespace std;

struct NodeRef {
  int rule, i;

  bool operator<(NodeRef const &other) const {
    if (rule == other.rule)  
      return i < other.i;
    return rule < other.rule;
  }
};

struct Descriptor {
  int node_id;
  NodeRef ref;

  bool operator<(Descriptor const &other) const {
    if (index == other.index)
      return ref < other.ref;
    return index < other.index;
  }
};


struct GSS {
  map<NodeRef, int> reverse_map;
  vector<NodeRef> nodes;
  vector<set<int>> parents;
  vector<set<int>> end_indices;

  int add_node(NodeRef ref) {
    if (reverse_map.count(ref))
      return -1;
    int idx = nodes.size();
    reverse_map[ref] = idx;
    nodes.push_back(ref);
    links.push_back(set<int>());
    return idx;
  }

  bool add_parent(NodeRef ref, NodeRef parent) {
    int from = reverse_map[ref];
    int to = reverse_map[parent];
    set<int> &refs(parents[from]);

    if (refs.count(to))
      return false;
    refs.insert(to);
  }

};

struct Op {
  virtual void operator()(string &data, GSS &gss, queue<Descriptor> &queue, NodeRef node, int index){}
};

struct MatchOp : public Op {
  MatchOp() {}
  void operator()(string &input, GSS &gss, queue<Descriptor> &queue, NodeRef node, int index) {
    int n = match(input, index);
    if (n >= 0) {
      
    }
  }

  virtual int match(string &input, int index) {return -1;}
}
  
struct MatchStringOp : public MatchOp {
  MatchStringOp(string token_) : token(token_) {}

  virtual int match(string &input, int index) {
    if (token.size() + index > input.size()) //doesnt fit
      return -1;
    string match = data.substr(index, token.size());
    if (match == token)
      return token.size();
    return -1;
  }
  
  string token;
};

struct MatchRangeOp : public MatchOp {
  MatchRangeOp(char a_, char b_) : a(a_), b(b_) {}

  virtual int match(string &input, int index) {
    if (index >= input.size())
      return -1;
    if (input[index] >= a && input[index] <= b)
      return 1;
    return -1;
  }

  char a, b;
};

struct SpawnOp : public Op {  
  SpawnOp() {}
  SpawnOp(int spawn_rule) {spawn_rules.push_back(spawn_rule); }
  SpawnOp(vector<int> spawn_rules_) : spawn_rules(spawn_rules_) {}

  void operator()(string &data, GSS &gss, queue<Descriptor> &queue, NodeRef node, int index) {
    for (int rule : spawn_rules)
      gss.add_link(node, NodeRef{rule, index});
  }

  void add_rule(int rule_id) {
    spawn_rules.push_back(rule_id);
  }
  
  vector<int> spawn_rules;
};

struct EndOp : public op {
  EndOp() {}
  void operator()(string &data, GSS &gss, queue<Descriptor> &queue, NodeRef node, int index) {
    if (index == data.size())
      throw "matched";
  }
};

struct Grammar {
  vector<Op*> ops;

  int add_rule(Op *rule) {
    int index = ops.size();
    ops.push_back(rule);
    return index;
  }

  void add_stop() {
    ops.push_back(0);
  }
};


//Compile stuff for building a base grammar
typedef map<string, vector<string>> GrammerDef;

Grammar compile(GrammerDef gramdef) {
  Grammar grammar;

  map<string, int> rule_map;


  //first pass, counting named rules
  int n_rules = 0;
  for (auto &rule_def : gramdef) {
    string name = rule_def.first;
    int rule_id = grammar.add_rule(new SpawnOp());
    rule_map[name] = rule_id;
    
    grammar.add_rule(new SpawnOp());
    grammar.add_stop();
  }

  for (auto &rule_def : gramdef) {
    string name = rule_def.first;
    vector<string> &spawn_strings = rule_def.second;
    int main_rule_nr = nr[name]; //index of main rule
    
    SpawnOp* rule_spawner = new SpawnOp();

    for (string desc : rule_def.second) { //for each spawn string
      //split the spawn string in elements
      istringstream iss(desc);
      vector<string> descriptions;
      copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(descriptions));

      for (size_t i(0); i < descriptions.size(); ++i) {
        string &name = descriptions[i];
	int rule_id = 0;
        if (rule_map.count(name)) { //a rule is matched
	  rule_id = grammar.add_rule(new SpawnOp(rule_map[name]));
        } else {  //its a new rule
          rule_id = grammar.add_rule(new MatchOp(name));
        }
	if (i == 0)
	  rule_spawner->add_rule(rule_id);
      }
      grammar.add_stop();
    }

    grammar.add_rule(rule_spawner);
    grammar.add_stop();
  }

  return grammar;
}

struct Parser {
  Parser(Grammar &grammar_) : grammar(grammar_) {
  }

  void parse(string input_) {
    input = input_;

    while (q.size())
      step();

  }

  void push_descriptor(Descriptor &descriptor) {
    q.push(descriptor);
  }

  void step() {
    Descriptor current = q.front();
    q.pop();
    (*grammar.ops[current.ref.rule])(data, gss, q, current.ref, current.index);
  }

  GSS gss;
  Grammar grammar;
  queue<Descriptor> q;
  string input;
};

int main(int argc, char **argv) {
  // assert(argc == 2);

  //string str(argv[1]);

  cout << "Starting" << endl;

  GrammerDef grammar_def;
  grammar_def["S"] = vector<string>{"A S d", "B S"};
  grammar_def["A"] = vector<string>{"a", "c"};
  grammar_def["B"] = vector<string>{"a", "b"};
  
  auto grammar = compile(grammar_def);

  Parser parser(grammar);
  parser.parse("aad");
}
