#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <map>
#include <string>
#include <set>
#include <queue>
#include <cassert>
#include <regex>
#include <re2/re2.h>

using namespace std;

struct Node {
  int rule, i;

  bool operator<(Node const &other) const {
    if (i == other.i)
      return rule < other.rule;
    return i < other.i;
  }

  bool operator>(Node const &other) const {
    if (i == other.i)
      return rule > other.rule;
    return i > other.i;
  }
};

ostream &operator<<(ostream &out, Node node) {
  return out << "[r" << node.rule << ",i" << node.i << "]";
}

struct Descriptor {
  int node_id;
  Node node;

  bool operator<(Descriptor const &other) const {
    return node < other.node;
  }

  bool operator>(Descriptor const &other) const {
    return node > other.node;
  }
};

ostream &operator<<(ostream &out, Descriptor d) {
  return out << "[ni" << d.node_id << ",r" << d.node.rule << ":i" << d.node.i << "]";
}

typedef priority_queue<Descriptor, vector<Descriptor>, greater<Descriptor> > DescriptorQueue;

struct GSS {
  map<Node, int> reverse_map;
  vector<Node> nodes;
  vector<set<int>> parents;
  vector<set<int>> end_indices;
  vector<set<int>> trace;
  
  int add_node(Node ref, set<int> node_parents = set<int>()) {
    if (reverse_map.count(ref))
      return -1;
    int idx = nodes.size();
    reverse_map[ref] = idx;
    nodes.push_back(ref);
    parents.push_back(node_parents);
    end_indices.push_back(set<int>());
    trace.push_back(set<int>());
    return idx;
  }

  bool has_ends(int nodeidx) {
    return end_indices[nodeidx].size();
  }

  Node operator[](int idx) {
    return nodes[idx];
  }

  int operator[](Node node) {
    if (reverse_map.count(node))
      return reverse_map[node];
    return -1;
  }
};

ostream &operator<<(ostream &out, GSS &gss) {
  for (size_t i(0); i < gss.nodes.size(); ++i) {
    out << i << ": r" << gss.nodes[i].rule << " i" << gss.nodes[i].i << "-";
    for (int e : gss.end_indices[i])
      out << e << ",";
    out << " p:";
    for (int p : gss.parents[i])
      out << p << ",";
    out << " t:";
    for (int t : gss.trace[i])
      cout << t << ",";
    out << endl;
  }

  return out;
}

struct Op;

struct Grammar {
  vector<Op*> ops;
  map<string, int> rule_map;
  string start_rule;

  //~Grammar();
  
  int add_rule(Op *rule) {
    int index = ops.size();
    ops.push_back(rule);
    return index;
  }

  void add_stop() {
    ops.push_back(0);
  }

  Op &operator[](int idx) {
    return *ops[idx];
  }

  int get_rule(string name) {
    return rule_map[name];
  }
};


void add_node(Node node, set<int> parents, GSS &gss, Grammar &grammar, DescriptorQueue &q, int from_node);

//the next helper function
void spawn_next(int nodeidx, int input_idx, GSS &gss, Grammar &grammar, DescriptorQueue &q, int from_node) {
  queue<int> node_queue;
  node_queue.push(nodeidx);
  
  while (node_queue.size()) {
    int idx = node_queue.front(); node_queue.pop();
    Node node = gss[idx];
    int rule = node.rule + 1;

    if (grammar.ops[rule] == 0) {
      set<int> &parents(gss.parents[idx]);
      for (int p : parents) {
	auto res = gss.end_indices[p].emplace(input_idx);
	//if (res.second) //insertion succeeded
	node_queue.push(p);
      }
      //for (int p : parents) {//parents were popped, add end indices

      //}
    } else {
      //cout << "adding " << Node{rule, input_idx} << endl;
      add_node(Node{rule, input_idx}, gss.parents[idx], gss, grammar, q, from_node);
    }
  }
}

void add_node(Node node, set<int> parents, GSS &gss, Grammar &grammar, DescriptorQueue &q, int from_node) {
  int idx = gss.add_node(node, parents);
  if (idx == -1) { //node exists
    idx = gss[node];
    set<int> &node_parents = gss.parents[idx];
    for (auto p : parents)
      node_parents.insert(p);
    if (gss.has_ends(idx)) {
      for (int e : gss.end_indices[idx]) {
	//cout << "adding from " << node << " e" << e << endl;
	spawn_next(idx, e, gss, grammar, q, from_node);

      }
    }
  } else {
    //spawn descriptor, could add set here to see if it was added already
    //cout << "adding " << Descriptor{idx, node} << endl;
    q.push(Descriptor{idx, node});
    gss.trace[idx].insert(from_node);
  }

}

//Ops are also the algorithm responsibles and have access to everything
struct Op {
  virtual ~Op(){}
  virtual void operator()(Node node, int node_idx, int index, string &input, Grammar &grammar, GSS &gss, DescriptorQueue &q){}
  virtual void print(ostream &out){}
};

ostream &operator<<(ostream &out, Op &op) {
  op.print(out);
  return out;
}

struct MatchOp : public Op {
  MatchOp() {}
  ~MatchOp(){}
  void operator()(Node node, int node_idx, int index, string &input, Grammar &grammar, GSS &gss, DescriptorQueue &q) {
    int n = match(input, index);
    if (n >= 0) {
      index += n;
      spawn_next(node_idx, index, gss, grammar, q, node_idx);
    }
  }

  virtual int match(string const &input, int index) {return -1;}
};
  
struct MatchStringOp : public MatchOp {
  MatchStringOp(string token_) : token(token_) {}
  ~MatchStringOp(){}
  
  virtual int match(string const &input, int index) {
    if (token.size() + index > input.size()) {//doesnt fit
      //cout << index << " doesn't fit" << endl;
      return -1;
    }
    //cout << "comparing " << token << " " << input << " " << index << endl;
    if (input.compare(index, token.size(), token) == 0) {
      cout << "matched: " << token << " at " << index << endl;
      return token.size();
    }
    return -1;
  }

  virtual void print(ostream &out) {
    out << "Match: [" << token << "]";
  }
  
  string token;
};

struct MatchRegexOp : public MatchOp {
  MatchRegexOp(string expr_) : expr(expr_), reg(expr_) {
    if (!reg.ok())
      throw reg.error();
    matches.resize(1); //we reuse vector for optimisation
  }

  ~MatchRegexOp(){}
  
  virtual int match(string const &input, int index) {
    if (index >= input.size())
      return -1;
    if (reg.Match(input, index, input.size(), RE2::ANCHOR_START, &matches[0], 1)) {
      //cout << "match " << matches[0].length() << " " << expr << " input:" << input.substr(index, index+10) << " i:" << index << endl;
	return matches[0].length();
    }
    return -1;
  }

  virtual void print(ostream &out) {
    out << "RegexMatch: [" << expr << "]";
  }
  
  RE2 reg;
  vector<re2::StringPiece> matches;
  string expr;
};

struct MatchRangeOp : public MatchOp {
  MatchRangeOp(char a_, char b_) : a(a_), b(b_) {}
  ~MatchRangeOp(){}
  
  virtual int match(string const &input, int index) {
    if (index >= input.size())
      return -1;
    if (input[index] >= a && input[index] <= b)
      return 1;
    return -1;
  }

    virtual void print(ostream &out) {
      out << "Match: [" << a << "-" << b << "]";
  }

  char a, b;
};
  
  
struct SpawnOp : public Op {  
  SpawnOp() {}
  ~SpawnOp(){}
  
  SpawnOp(int spawn_rule) {spawn_rules.push_back(spawn_rule); }
  SpawnOp(vector<int> spawn_rules_) : spawn_rules(spawn_rules_) {}

  void operator()(Node node, int node_idx, int index, string &input, Grammar &grammar, GSS &gss, DescriptorQueue &q) {
    for (int rule : spawn_rules) {
      Node node{rule, index};
      add_node(node, set<int>{node_idx}, gss, grammar, q, node_idx);
    }
  }

  void add_rule(int rule_id) {
    spawn_rules.push_back(rule_id);
  }

  virtual void print(ostream &out) {
    out << "Spawn: ";
    for (auto r : spawn_rules) out << r << ",";
  }

  
  vector<int> spawn_rules;
};

struct EndOp : public Op {
  EndOp() {}
  ~EndOp(){}

  void operator()(Node node, int node_idx, int index, string &input, Grammar &grammar, GSS &gss, DescriptorQueue &q) {
    //cout << "End Op " << index << endl;
    if (index == input.size()) {
      cout << "matched" << endl;
      vector<int> traceback;
      int cur = node_idx;
      while (true) {
	traceback.push_back(cur);
	if (!gss.trace[cur].size())
	  break;
	cur = *gss.trace[cur].begin();
      }
      for (auto it = traceback.rbegin(); it != traceback.rend(); it++)
	cout << gss.nodes[*it].i << "-r" << gss.nodes[*it].rule << " ";
      cout << endl;
    }
  }

  virtual void print(ostream &out) {
    out << "End";
  }
};

ostream &operator<<(ostream &out, Grammar &grammar) {
  int c(0);
  for (auto op : grammar.ops) {
    if (op)
      cout << c << " " << (*op) << endl;
    else
      cout << c << " 0" << endl;
    ++c;
  }
  return out;
}

/*Grammar::~Grammar() {
  int c(0);
  for (auto &op : ops) {
    cout << c++ << " " << op << endl;
    if (op)
      delete op;
  }
  }*/

//Compile stuff for building a base grammar
typedef map<string, vector<string>> GrammerDef;

Grammar compile(GrammerDef gramdef, string start_rule) {
  Grammar grammar;
  grammar.start_rule = start_rule;

  //first pass, counting named rules
  int n_rules = 0;
  for (auto &rule_def : gramdef) {
    string name = rule_def.first;
    int rule_id = grammar.add_rule(new SpawnOp());
    grammar.rule_map[name] = rule_id;
    grammar.add_stop();
  }

  //add the root rule
  int start_rule_id = grammar.get_rule(grammar.start_rule);
  int root_rule = grammar.add_rule(new SpawnOp(start_rule_id));
  grammar.add_rule(new EndOp());
  grammar.add_stop();
  grammar.rule_map["$"] = root_rule;
  
  for (auto &rule_def : gramdef) {
    string name = rule_def.first;
    vector<string> &spawn_strings = rule_def.second;
    int main_rule_nr = grammar.get_rule(name); //index of main rule
    
    SpawnOp &rule_spawner = reinterpret_cast<SpawnOp&>(*grammar.ops[main_rule_nr]);

    for (string desc : rule_def.second) { //for each spawn string
      //split the spawn string in elements
      istringstream iss(desc);
      vector<string> descriptions;
      copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(descriptions));

      for (size_t i(0); i < descriptions.size(); ++i) {
        string &name = descriptions[i];
	int rule_id = 0;
        if (grammar.rule_map.count(name)) { //a rule is matched
	  rule_id = grammar.add_rule(new SpawnOp(grammar.get_rule(name)));
        } else if(name.size() == 3 && name.find("-") == 1) {
	  rule_id = grammar.add_rule(new MatchRangeOp(name[0], name[2]));
	} else {  //its a new rule
          //rule_id = grammar.add_rule(new MatchStringOp(name));
	  rule_id = grammar.add_rule(new MatchRegexOp(name));
        }
	if (i == 0)
	  rule_spawner.add_rule(rule_id);
      }
      grammar.add_stop();
    }
  }

  return grammar;
}

struct Parser {
  Parser(Grammar &grammar_) : grammar(grammar_) {
    int root_rule = grammar.get_rule("$");
    Node root_node = Node{root_rule, 0};
    int root_node_idx = gss.add_node(Node{root_rule, 0});
    q.push(Descriptor{root_node_idx, root_node});
  }

  void parse(string input_) {
    input = input_;

    while (q.size()) {
      step();
    }

  }

  void push_descriptor(Descriptor &descriptor) {
    q.push(descriptor);
  }

  void step() {
    Descriptor head = q.top();
    q.pop();
    //if (head.node.i % 100 == 0) {
    cout << "i: " << head.node.i << " r:" << head.node.rule << " c: " << input[head.node.i] << endl;
    cout << "nodes: " << gss.nodes.size() << endl;
    cout << "q: " << q.size() << endl;
      //}
    //cout << head << endl;
    (*grammar.ops[head.node.rule])(head.node, head.node_id, head.node.i, input, grammar, gss, q);
  }

  GSS gss;
  Grammar grammar;
  DescriptorQueue q;
  string input;
};

int main(int argc, char **argv) {
  assert(argc == 2);
  string filename(argv[1]);

  std::ifstream t(filename.c_str());
  t.seekg(0, std::ios::end);
  size_t size = t.tellg();
  std::string buffer(size, ' ');
  t.seekg(0);
  t.read(&buffer[0], size);
  
  cout << "Starting" << endl;

  GrammerDef grammar_def;
  grammar_def["S"] = vector<string>{"blocks [\\n]*"};
  grammar_def["blocks"] =  vector<string>{"block", "blocks return returns block"};
  grammar_def["block"] = vector<string>{"header", "paragraph"};
  grammar_def["header"] = vector<string>{"words return ===[=]*"};
  grammar_def["paragraph"] = vector<string>{"words", "paragraph return words"};
  grammar_def["words"] = vector<string>{"word", "words space word"};
  
  grammar_def["space"] = vector<string>{"[[:blank:]]+", "white"};
  grammar_def["word"] = vector<string>{"[[:graph:]]+"};
  grammar_def["return"] = vector<string>{"\\n"};
  grammar_def["returns"] = vector<string>{"[\\n]+"};
  //grammar_def["S"] = vector<string>{"def white name", "def white number"};
  //grammar_def["name"] = vector<string>{"[a-zA-Z][a-zA-Z0-9]+"};
  //grammar_def["number"] = vector<string>{"[0-9]+"};
  //grammar_def["white"] = vector<string>{"[[:space:]\\t\\n]+"};
  
  auto grammar = compile(grammar_def, "S");
  cout << grammar << endl;
  //return 0;
  cout << "starting parse" << endl;
  Parser parser(grammar);
  
  parser.parse(buffer);
  
  //cout << parser.gss << endl;
  cout << "n nodes: " << parser.gss.nodes.size() << endl;
  
}
