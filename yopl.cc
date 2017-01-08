#ifndef __YOPL_H__
#define __YOPL_H__

#include <vector>
#include <set>
#include <map>
#include <queue>
#include <re2/re2.h>
#include <string>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <fstream>
#include <iostream>


using namespace std;

const bool DEBUG(false);

struct NodeIndex {
  int cursor, rule;
  int nodeid;
  
  bool operator<(NodeIndex const &other) const {
    if (cursor != other.cursor)
      return cursor < other.cursor;
    return rule > other.rule;
  }

  bool operator>(NodeIndex const &other) const {
    if (cursor != other.cursor)
      return cursor > other.cursor;
    return rule < other.rule;
  }
};

ostream &operator<<(ostream &out, NodeIndex &ni) {
  return out << "c" << ni.cursor << " r" << ni.rule << " i" << ni.nodeid;
}


struct Head {
  int cursor, rule;
  int node;
  
  bool operator<(Head const &other) const {
    if (cursor != other.cursor)
      return cursor < other.cursor;
    return rule > other.rule;
  }

  bool operator>(Head const &other) const {
    if (cursor != other.cursor)
      return cursor > other.cursor;
    return rule < other.rule;
  }
};

ostream &operator<<(ostream &out, Head &head) {
  return out << "c" << head.cursor << " r" << head.rule << " i" << head.node;
}


enum RuleType {
  OPTION = 0,
  MATCH = 1,
  RETURN = 2,
  END = 3
};

ostream &operator<<(ostream &out, RuleType &t) {
  switch (t) {
  case OPTION:
    return out << "OPTION"; break;
  case MATCH:
    return out << "MATCH"; break;
  case RETURN:
    return out << "RETURN"; break;
  case END:
    return out << "END"; break;
  }
}

struct RuleSet {
  std::vector<std::string> names;
  std::vector<RuleType> types;
  std::vector<std::vector<int>> arguments;
  std::vector<RE2*> matcher;
  std::vector<int> returns; // points to return (or end) point of each rule, needed to place crumbs
  
  RuleSet(){}
  
  RuleSet(string filename) {
    ifstream infile(filename.c_str());

    enum Mode {
      BLANK = 0,
      READ = 1,
      ESCAPE = 2
    };

    string line;
    
    map<string, vector<vector<string>>> rules;
    
    while (getline(infile, line)) {
      cout << line << endl;
      istringstream iss(line);

      string name;
      iss >> name;
      if (name.size() == 0)
	continue;
      cout << "name: " << name << endl;

      
      Mode mode(BLANK);
      string item;
      vector<string> curitems;
      vector<vector<string>> options;
      
      while (true) {
	char c = iss.get();
	if (c == EOF) {
	  if (item.size())
	    curitems.push_back(item);
	  options.push_back(curitems);
	  rules[name] = options;
	  break;
	}
	cout << c;
	if (mode == BLANK) {
	  if (c == '|') { //a new set
	    //add current items to vector
	    options.push_back(curitems);
	    curitems.clear();
	    item.clear();
	  }
	  else if (c == ' ')
	    ;
	  else if (c == '\'')
	    mode = ESCAPE;
	  else {
	    item += c;
	    mode = READ;
	  }
	}
	else if (mode == READ) {
	  if (c == ' ') {
	    //add item to set
	    curitems.push_back(item);
	    item.clear();
	    mode = BLANK;
	  } else
	    item += c;
	}
	else if (mode == ESCAPE) {
	  if (c == '\'') {
	    //add item to set
	    curitems.push_back(item);
	    item.clear();
	    mode = BLANK;
	  } else
	    item += c;
	}
      }      
      cout << endl;      
    }


    //Add the rules
    add_option("ROOT", vector<int>{2});
    add_end();

    typedef pair<int, string> so_pair;
    multimap<int, string> search_option; //backsearching the option calls afterwards
    map<string, int> rule_pos;
    
    for (auto r : rules) {
      string name = r.first;
      vector<vector<string>> &options = r.second;

      int start = size();
      rule_pos[name] = start;

      cout << name << " " << start << endl;

      if (options.size() == 1) { //we dont need an option
	auto &expressions = options[0];
	for (auto &exp : expressions) {
	  if (rules.count(exp)) { //refers to a rule
	    cout << size() << " " << exp << endl;
	    search_option.insert(so_pair(size(), exp));
	    add_option();
	  } else { //a matcher
	    add_match(exp);
	  }
	}
	add_ret();
	
      } else { //we need an option
	add_option();
	add_ret();

	for (auto o : options) {
	  int op_start = size();
	  if (o.size() == 1 && rules.count(o[0])) {
	    search_option.insert(so_pair(start, o[0]));
	  } else {
	    arguments[start].push_back(size());
	    for (auto exp : o) {
	      if (rules.count(exp)) { //refers to a rule
		search_option.insert(so_pair(size(), exp));
		add_option();
	      } else { //a matcher
		add_match(exp);
	      }
	    }
	    
	    add_ret();
	  }
	}
      }
    }
    
    // back reference option calls
    for (auto kv : search_option) {
      int option_pos = kv.first;
      string call_name = kv.second;
      arguments[option_pos].push_back(rule_pos[call_name]);
      cout << "adding " << option_pos << " " << call_name << " " << rule_pos[call_name] << endl;
    }
    
    //set names
    for (auto kv : rule_pos)
      names[kv.second] = kv.first;

    //set returns
    int last_ret(0);
    for (int i(types.size()-1); i > 0; --i) {
      if (types[i] == RETURN || types[i] == END)
	last_ret = i;
      returns[i] = last_ret;
    }
    

    //print rules
    for (int i(0); i < types.size(); ++i) {
      cout << i << ": [" << types[i] << "] ";
      cout << names[i] << " :";
      if (types[i] == OPTION)
	for (auto i : arguments[i])
	  cout << i << ",";
      if (types[i] == MATCH)
	cout << "'" << matcher[i]->pattern() << "'";
      
      cout << endl;
    }
  }
     
  void add_ret() {
    names.push_back("");
    types.push_back(RETURN);
    arguments.push_back(vector<int>(0,0));
    matcher.push_back(0);
    returns.push_back(0);
  }

  void add_end() {
    names.push_back("");
    types.push_back(END);
    arguments.push_back(vector<int>(0,0));
    matcher.push_back(0);
    returns.push_back(0);
  }

  void add_option(string name, vector<int> spawn = vector<int>()) {
    names.push_back(name);
    types.push_back(OPTION);
    arguments.push_back(spawn); //call S
    matcher.push_back(0);
    returns.push_back(0);
  }

  void add_option(vector<int> spawn = vector<int>()) {
    add_option("", spawn);
  }
  
  void add_match(string name, string matchstr) {
    names.push_back(name);
    types.push_back(MATCH);
    arguments.push_back(vector<int>(0,0));
    matcher.push_back(new RE2(matchstr));
    returns.push_back(0);
  }

  void add_match(string matchstr) {
    add_match("", matchstr);
  }
  
  int size() {
    return types.size();
  }
  
};

int match(RE2 &matcher, string &str, int pos = 0) {
  re2::StringPiece match;
  if (matcher.Match(str, pos, str.size(), RE2::ANCHOR_START, &match, 1)) {
    if (DEBUG) cout << "Matched " << match.length() << endl;
    return match.length();
  }
  //failed
  return -1;
}

struct Parser {
  RuleSet ruleset;

  set<NodeIndex> stack;
  vector<NodeIndex> nodes;
  vector<int> properties;
  vector<set<int>> parents;
  vector<set<int>> ends;
  vector<set<int>> crumbs;

  Parser(string gram_file) : ruleset(gram_file) {
    
  }

  void add_node(NodeIndex node, int prop_node, int parent = -1, int crumb = -1) { //does not check whether it exists
    stack.insert(node);
    nodes.push_back(node);
    properties.push_back(prop_node);
    parents.push_back(parent == -1 ? set<int>() : set<int>{parent});
    crumbs.push_back(crumb == -1 ? set<int>() : set<int>{crumb});
    ends.push_back(set<int>());
  }
  
  int parse(string input_file) {
    std::ifstream infile(input_file);
    std::string buffer((std::istreambuf_iterator<char>(infile)),
		       std::istreambuf_iterator<char>());

    priority_queue<Head> heads;
    
    //add the ROOT node
    add_node(NodeIndex{0, 0, 0}, 0);
  
    //add ROOT head
    heads.push(Head{0, 0, 0});

    //Start Parsing
    int end_node(0);
    while(heads.size() && end_node == 0) {
      Head head = heads.top();
      if (DEBUG) cout << "head: " << head << " " << ruleset.types[head.rule] << endl;

      /*if (nodes.size() > 1000) {
	cout << "nodes: ";
	for (int n(0); n < nodes.size(); ++n) {
	cout << nodes[n].cursor << "r" << nodes[n].rule << " ";
	}
	cout << endl;
	return -1;
	}*/
    
      heads.pop();
      switch (ruleset.types[head.rule]) {
      case END:
	if (head.cursor == buffer.size()) {
	  end_node = head.node;
	  continue;
	}
	break;
      case MATCH:
	{
	  int n = head.node;
	  int cur = head.cursor;
	  int r = head.rule;
	  int m = match(*ruleset.matcher[r], buffer, cur);
	  if (DEBUG) cout << "Match rule: '" << ruleset.matcher[r]->pattern() << "' [" << cur<< "] matched " << m << endl;
	
	  if (m < 0) break; //no match

	  //allright, add the node
	  auto new_node = NodeIndex{cur + m, r+1, (int)nodes.size()};

	  if (DEBUG) cout << "adding " << new_node << endl;
	  add_node(new_node, properties[n], -1, n);
	  
	  heads.push(Head{new_node.cursor, new_node.rule, new_node.nodeid});
	}
	break;
      case OPTION:
	{
	  int n = head.node;
	  int cur = head.cursor;
	  int r = head.rule;
	  vector<int> &args = ruleset.arguments[r];

	  for (int new_r : args) {
	    NodeIndex ni{cur, new_r, (int)nodes.size()};
	    if (stack.count(ni)) { //node exists
	      int id = stack.find(ni)->nodeid;

	      parents[id].insert(n);
	      crumbs[id].insert(n);

	      //if already has ends, add nexts
	      set<int> ends_copy = ends[id];
	      for (int e : ends_copy) {
		auto new_node = NodeIndex{e, r + 1, (int)nodes.size()};
		NodeIndex crumb_node{e, ruleset.returns[new_r], 0}; //should exist
		int crumb_id = stack.find(crumb_node)->nodeid;
	      
		if (stack.count(new_node)) {
		  int existing_id = stack.find(new_node)->nodeid;
		  int existing_prop = properties[existing_id];
		  int our_prop = properties[n];

		  parents[existing_prop].insert(parents[our_prop].begin(), parents[our_prop].end());
		  crumbs[existing_id].insert(crumb_id);
		} else { //add node
		  if (DEBUG) cout << "adding " << new_node << endl;
		  add_node(new_node, properties[n], -1, crumb_id);
		
		  heads.push(Head{new_node.cursor, new_node.rule, new_node.nodeid});
		}
	      }
	    } else {
	      //create node
	      auto new_node = ni;

	      if (DEBUG) cout << "adding " << new_node << endl;
	      add_node(new_node, new_node.nodeid, n, n);

	      heads.push(Head{new_node.cursor, new_node.rule, new_node.nodeid});
	    }
	  }
	}
	break;
      case RETURN:
	{
	  int properties_node = properties[head.node];
	  int cur = head.cursor;
	  ends[properties_node].insert(cur);
	  set<int> par = parents[properties_node];
	
	  for (int p : par) {
	    auto new_node = NodeIndex{cur, nodes[p].rule + 1, (int)nodes.size()};

	  
	    if (stack.count(new_node) && ruleset.types[new_node.rule] != RETURN) {
	      int existing_id = stack.find(new_node)->nodeid;
	      int existing_prop = properties[existing_id];
	      int parent_prop = properties[p];
	      
	      parents[existing_prop].insert(parents[parent_prop].begin(), parents[parent_prop].end());
	      crumbs[existing_id].insert(head.node);
	      
	    } else {
	      if (DEBUG) cout << "adding " << new_node << endl;
	      add_node(new_node, properties[p], -1, head.node);
	      
	      heads.push(Head{new_node.cursor, new_node.rule, new_node.nodeid});
	    }
	  }
	}
	break;
      }
    }

    //post processing
    vector<int> active_nodes;
    if (end_node) {
      set<int> seen_nodes;
      queue<int> q;
      q.push(end_node);
      //int n = end_node;

      while (!q.empty()) {
	int n = q.front();
	if (!seen_nodes.count(n)) {
	  active_nodes.push_back(n);
	  seen_nodes.insert(n);
	  
	  for (int c : crumbs[n])
	    q.push(c);
	}
	q.pop();
      }

      reverse(active_nodes.begin(), active_nodes.end());


      //run through active nodes, filtering and ending
      set<string> filter_set;
      filter_set.insert("ws");

      vector<string> names;
      vector<int> starts;
      vector<int> ends;
      vector<set<int>> children;

      map<int, int> node_map;
      int n_parse_nodes(0);

      bool last_was_match(false); int last_n(0); //little hacky, matches dont return
      for (int n : active_nodes) {
	NodeIndex &node = nodes[n];
	cout << "active: " << node << endl;

	if (ruleset.names[node.rule].size() && !filter_set.count(ruleset.names[node.rule])) {
	  node_map[n] = n_parse_nodes++;
	  names.push_back(ruleset.names[node.rule]);
	  starts.push_back(node.cursor);
	  ends.push_back(-1);
	  children.push_back(set<int>());
	  
	  //make link from parent to child
	  for (int p : parents[properties[n]])
	    if (node_map.count(p))
	      children[node_map[p]].insert(node_map[n]);
	}

	
	//set end for the matching rule of return
	if (ruleset.types[node.rule] == RETURN)
	  ends[node_map[properties[n]]] = node.cursor;
	  //for (int p : parents[properties[n]])
	  //if (node_map.count(p))
	  //ends[node_map[p]] = node.cursor;

	//set ends for MATCH nodes
	if (last_was_match) {
	  ends[node_map[last_n]] = node.cursor;
	  last_was_match = false;
	}
	if (ruleset.types[node.rule] == MATCH) {
	  last_was_match = true;
	  last_n = n;
	}

	
	/*
	cout << "node: " << n << " cursor: " << nodes[n].cursor << " "  << ruleset.types[nodes[n].rule] << endl;
	//int p = properties[n];
	if (parents[n].size()) {
	  cout << "parents: ";
	  for (auto p : parents[n])
	    cout << p << " ";
	  cout << endl;
	}
	*/
      }

      ofstream dotfile("parsetree.dot");

      dotfile << "digraph parsetree {" << endl;      
      for (int i(0); i < n_parse_nodes; ++i) {
	if (ends[i] >= 0) {//not all active nodes are valid; if they are unended they never matched completely
	  cout << names[i] << " " << starts[i] << "-" << ends[i] << " [" << buffer.substr(starts[i],  ends[i] - starts[i]) << "]" << endl;
	  dotfile << "node_" << i << " [label=\"" << names[i] << " [" << buffer.substr(starts[i],  ends[i] - starts[i]) << "]\"];" << endl;
	  for (int c : children[i]) {
	    if (ends[c] != -1)
	      dotfile << "node_" << i << " -> node_" << c << ";" << endl;
	  }
	}
      }
      dotfile << "}" << endl;
      cout << "SUCCESS" << endl;
      return 0;
    } else {
      cout << "FAILED" << endl;
      return 1;
    }

  }
};


int main(int argc, char **argv) {
  cout << "yopl" << endl;
  Parser parser("gram.txt");
  return parser.parse("input.txt");
}

#endif
