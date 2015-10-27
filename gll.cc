#include <iostream>
#include <vector>
#include <map>
#include <string>

using namespace std;

struct RuleRef {
  int rule, i;

  bool operator<(RuleRef &const other) const {
    if (rule == other.rule)
      return i < other.i;
    return rule < other.rule;
  }
};

struct Descriptor {
  int index;
  RuleRef ref;

  bool operator<(Descriptor &const other) const {
    if (index == other.index)
      return ref < other.ref;
    return index < other.index;
  }
};

struct GSS {
  map<RuleRef, int> node_map;
  vector<RuleRef> nodes;
  vector<set<int>> links;
  

  bool add_node(RuleRef ref) {
    if (node_map.count(ref))
      return false;
    node_map[ref] = nodes.size();
    nodes.push_back(ref);
    links.push_back(set<int>())
  }

  bool add_link(RuleRef ref, RuleRef parent) {
    int from = node_map[ref];
    int to = node_map[parent];
    set<int> &refs(links[from]);

    if (set.count(to))
      return false;
    refs.insert(to);
  }

};

struct Op {
  virtual operator()(){}
};

struct MatchOp : public Op {
  MatchOp(string a_) : a(a_) {}
    operator()(string &data, GSS &gss, RuleRef node, int index) {
      if (data.substr(index, index+len(a)) == a)
	gss.add_descriptor(Descriptor{index+1, node});
    }
  
  string a;
};

struct MatchRangeOp : public Op {
  MatchRangeOp(char a_, char b_) : a(a_), b(b_) {}
  operator()(string &data, GSS &gss, RuleRef node, int index) {
    if (data[index] >= a && data[index] <= b)
      gss.add_descriptor(Descriptor{index+1, node});
  }

  char a, b;
};

struct SpawnOp {
  SpawnOp(vector<int> spawn_rules_) : spawn_rules(spawn_rules_)
  operator()(string &data, GSS &gss, RuleRef node, int index) {
    for (int rule : spawn_rules)
      gss.add_link(node, RuleRef{rule, index});
  }

  vector<int> spawn_rules;
};

struct SequenceOp {
  SpawnOp(){}
  SpawnOp(vector<int> spawn_rules_) : spawn_rules(spawn_rules_)
  operator()(string &data, GSS &gss, RuleRef node, int index) {
    for (int rule : spawn_rules)
      gss.add_link(node, RuleRef{rule, index});
  }

  vector<int> spawn_rules;
};

struct Grammar {
  vector<Op*> ops;

  int add_rule(Ops *rule) {
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
    map[name] = map.size()
  }

  //start subrule counter
  int subrule = nr.size();

  for (auto &rule_def : gramdef) {
    int main_rule_nr = nr[rule_def.first]; //index of main rule
    

    for (string desc : rule_def.second) {

      vector<Ops> ops;

      istringstream iss(desc);
      vector<string> descriptions;
      copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(descriptions));

      for (auto &desc : descriptions) {
        copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(name));

        ((SpanOp*)grammar.ops[rule_nr]).spawn_rules.push_back(subrule);

        for (size_t i(0); i < names.size(); ++i) {
          if (names.count(name)) {
            if (i != names.size() - 1)
              grammar.ops[subrule] = new SequenceOp(vector<int>{nr[sub], subrule + 1});
            else
              grammar.ops[subrule] = new SequenceOp(vector<int>{nr[sub]});
          } else {
            grammar.ops[subrule] = new MatchOp(name);
          }
          subrule++;
        }
      }
    }
  }
}

struct Parser {
  Parser(Grammar &grammar_) : grammar(grammar_) {
  }

  void parse(string input) {

  }

  GSS gss;
  Grammar grammar;
};

int main(int argc, char **argv) {
  assert(argc == 2);

  string str(argv[1]);

  cout << "Starting" << endl;

  GrammerDef grammar_def;
  grammar_def["S"] = vector<string>{"ASd", "BS"};
  grammar_def["A"] = vector<string>{"a", "c"};
  grammar_def["B"] = vector<string>{"a", "b"};
  
  auto grammar = compile(grammar_def);

  Parser parser(grammar);
  parser.parse("aad");
}
