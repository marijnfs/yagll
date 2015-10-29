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
  int index;
  NodeRef ref;

  bool operator<(Descriptor const &other) const {
    if (index == other.index)
      return ref < other.ref;
    return index < other.index;
  }
};


struct GSS {
  map<NodeRef, int> node_map;
  vector<NodeRef> nodes;
  vector<set<int>> links;
  

  bool add_node(NodeRef ref) {
    if (node_map.count(ref))
      return false;
    node_map[ref] = nodes.size();
    nodes.push_back(ref);
    links.push_back(set<int>());
  }

  bool add_link(NodeRef ref, NodeRef parent) {
    int from = node_map[ref];
    int to = node_map[parent];
    set<int> &refs(links[from]);

    if (refs.count(to))
      return false;
    refs.insert(to);
  }

};

struct Op {
  virtual void operator()(string &data, GSS &gss, queue<Descriptor> &queue, NodeRef node, int index){}
};

struct MatchOp : public Op {
  MatchOp(string a_) : a(a_) {}
    void operator()(string &data, GSS &gss, queue<Descriptor> &queue, NodeRef node, int index) {
      if (data.substr(index, index + a.size()) == a)
      	queue.push(Descriptor{index+1, node});
    }
  
  string a;
};

struct MatchRangeOp : public Op {
  MatchRangeOp(char a_, char b_) : a(a_), b(b_) {}
  void operator()(string &data, GSS &gss, queue<Descriptor> &queue, NodeRef node, int index) {
    if (data[index] >= a && data[index] <= b)
      queue.push(Descriptor{index+1, node});
  }

  char a, b;
};

struct SpawnOp : public Op {  
  SpawnOp() {}

  SpawnOp(vector<int> spawn_rules_) : spawn_rules(spawn_rules_) {}
  void operator()(string &data, GSS &gss, queue<Descriptor> &queue, NodeRef node, int index) {
    for (int rule : spawn_rules)
      gss.add_link(node, NodeRef{rule, index});
  }

  vector<int> spawn_rules;
};

struct SequenceOp : public Op {
  SequenceOp(){}
  SequenceOp(vector<int> spawn_rules_) : spawn_rules(spawn_rules_){}
  void operator()(string &data, GSS &gss, queue<Descriptor> &queue, NodeRef node, int index) {
    for (int rule : spawn_rules)
      gss.add_link(node, NodeRef{rule, index});
  }

  vector<int> spawn_rules;
};

struct Grammar {
  vector<Op*> ops;

  int add_rule(Op *rule) {
    int index = ops.size();
    ops.push_back(rule);
    return index;
  }
};

typedef map<string, vector<string>> GrammerDef;



Grammar compile(GrammerDef gramdef) {
  Grammar grammar;

  set<string> names;
  map<string, int> nr;


  //first pass, counting named rules
  int n_rules = 0;
  for (auto &ops : gramdef) {
    names.insert(ops.first);
    grammar.ops.push_back(new SpawnOp());
  }

  //populating name to index map
  for (auto &name : names) {
    nr[name] = nr.size();
  }

  //start subrule counter
  int subrule = nr.size();

  for (auto &rule_def : gramdef) {
    int main_rule_nr = nr[rule_def.first]; //index of main rule
    
    SpawnOp* rule_spawner = new SpawnOp();

    for (string desc : rule_def.second) {
      istringstream iss(desc);
      vector<string> descriptions;
      copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(descriptions));

      rule_spawner->spawn_rules.push_back(subrule);
      
      grammar.ops.push_back(0);
      for (size_t i(0); i < descriptions.size(); ++i) {
        string &name = descriptions[i];
        if (names.count(name)) { //a rule is matched
          if (i != names.size() - 1) //if not last
              grammar.ops[subrule] = new SequenceOp(vector<int>{nr[name], subrule + 1});
            else
              grammar.ops[subrule] = new SequenceOp(vector<int>{nr[name]});
        } else {  //its a new rule
          grammar.ops[subrule] = new MatchOp(name);
        }
        subrule++;
      }
    }

    grammar.ops[main_rule_nr] = rule_spawner;
  }

  return grammar;
}

struct Parser {
  Parser(Grammar &grammar_) : grammar(grammar_) {
  }

  void parse(string input) {
    data = input;

    while (q.size())
      step();

  }

  void add_descriptor(Descriptor &descriptor) {
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
  string data;
};

int main(int argc, char **argv) {
  // assert(argc == 2);

  //string str(argv[1]);

  cout << "Starting" << endl;

  GrammerDef grammar_def;
  grammar_def["S"] = vector<string>{"ASd", "BS"};
  grammar_def["A"] = vector<string>{"a", "c"};
  grammar_def["B"] = vector<string>{"a", "b"};
  
  auto grammar = compile(grammar_def);

  Parser parser(grammar);
  parser.parse("aad");
}
