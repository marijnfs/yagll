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
  int cursor, rule, depth;
  int node;
  
  bool operator<(Head const &other) const {
    if (cursor != other.cursor)
      return cursor < other.cursor;
    if (depth != other.depth)
      return depth < other.depth;
    return rule > other.rule;
  }

  bool operator>(Head const &other) const {
    if (cursor != other.cursor)
      return cursor > other.cursor;
    if (depth != other.depth)
      return depth > other.depth;
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
  std::vector<int> return_id; // points to return (or end) point of each rule, needed to place crumbs
  
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
    
    map<int, string> search_option; //backsearching the option calls afterwards
    map<string, int> rule_pos;
    
    for (auto r : rules) {
      string name = r.first;
      vector<vector<string>> &options = r.second;
      rule_pos[name] = size();

      int start = size();
      if (options.size() == 1) { //we dont need an option
	for (auto exp : options[0]) {
	  if (rules.count(exp)) { //refers to a rule
	    search_option[size()] = exp;
	    add_option();
	  } else { //a matcher
	    add_match(exp);
	  }
	}
	return_id[start] = size(); //set start rule to end
	add_ret();
      } else { //we need an option
	add_option();
	return_id[start] = size();
	add_ret();

	for (auto o : options) {
	  arguments[start].push_back(size());
	  int op_start = size();
	  for (auto exp : o) {
	    if (rules.count(exp)) { //refers to a rule
	      search_option[size()] = exp;
	      add_option();
	    } else { //a matcher
	      add_match(exp);
	    }
	  }
	  return_id[op_start] = size();
	  add_ret();
	}
      }
    }
    
    // back reference option calls
    for (auto kv : search_option) {
      int option_pos = kv.first;
      string call_name = kv.second;
      arguments[option_pos].push_back(rule_pos[call_name]);
    }
    
    //set names
    for (auto kv : rule_pos)
      names[kv.second] = kv.first;


    //print rules
    for (int i(0); i < types.size(); ++i) {
      cout << i << ": [" << types[i] << "] ";
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
    return_id.push_back(0);
  }

  void add_end() {
    names.push_back("");
    types.push_back(END);
    arguments.push_back(vector<int>(0,0));
    matcher.push_back(0);
    return_id.push_back(0);
  }

  void add_option(string name, vector<int> spawn = vector<int>()) {
    names.push_back(name);
    types.push_back(OPTION);
    arguments.push_back(spawn); //call S
    matcher.push_back(0);
    return_id.push_back(0);
  }

  void add_option(vector<int> spawn = vector<int>()) {
    add_option("", spawn);
  }
  
  void add_match(string name, string matchstr) {
    names.push_back(name);
    types.push_back(MATCH);
    arguments.push_back(vector<int>(0,0));
    matcher.push_back(new RE2(matchstr));
    return_id.push_back(0);
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

int main(int argc, char **argv) {
  cout << "yopl" << endl;
  set<NodeIndex> stack;
  vector<NodeIndex> nodes;
  vector<int> properties;
  vector<set<int>> parents;
  vector<set<int>> ends;
  vector<set<int>> crumbs;
  
  
  priority_queue<Head> heads;
  
  std::ifstream infile("input.txt");
  std::string buffer((std::istreambuf_iterator<char>(infile)),
		  std::istreambuf_iterator<char>());
  
  RuleSet ruleset("gram.txt");

  //add the ROOT node
  stack.insert(NodeIndex{0, 0, 0});
  nodes.push_back(NodeIndex{0, 0, 0});
  properties.push_back(0);
  parents.push_back(set<int>());
  ends.push_back(set<int>());
  crumbs.push_back(set<int>());
  
  
  //add ROOT head
  heads.push(Head{0, 0, 0, 0});

  //Start Parsing
  while(heads.size()) {
    //for (auto p : parents)
    //  cout << p.size() << ' ';
    //cout << endl;
    //for (auto p : properties)
    //  cout << p << ' ';
    //cout << endl;
    Head head = heads.top();
    /*if (DEBUG)*/ cout << "head: " << head << " " << ruleset.types[head.rule] << endl;

    heads.pop();
    switch (ruleset.types[head.rule]) {
    case END:
      if (head.cursor == buffer.size()) {
	cout << "SUCCESS" << endl;
	return 0;
      }
      break;
    case RETURN:
      {
	int properties_node = properties[head.node];
	int cur = head.cursor;
	set<int> par = parents[properties_node];
	ends[properties_node].insert(cur);
	
	for (int p : par) {
	  auto new_node = NodeIndex{cur, nodes[p].rule+1, (int)nodes.size()};

	  //TODO: probably should make properties a vec of sets, multiple parents can cause any node in the middle
	  cout << p << endl;
	  if (stack.count(new_node)) { // && properties[stack.find(new_node)->nodeid] == properties[p])
	    int existing_id = stack.find(new_node)->nodeid;
	    int existing_prop = properties[existing_id];
	    int parent_prop = properties[p];
	    if (existing_prop != parent_prop) {
	      parents[existing_prop].insert(parents[parent_prop].begin(), parents[parent_prop].end());
	      properties[p] = properties[existing_prop];

	      //test: //should instead check ends and add them directly?
	      auto new_node = nodes[existing_id];
	      heads.push(Head{new_node.cursor, new_node.rule, head.depth, new_node.nodeid});
	    }	    

	  } else {
	    if (DEBUG) cout << "adding " << new_node << endl;
	    stack.insert(new_node);
	    nodes.push_back(new_node);
	    properties.push_back(properties[p]);
	    parents.push_back(set<int>());
	    ends.push_back(set<int>());
	    crumbs.push_back(set<int>{head.node});
	    
	    heads.push(Head{new_node.cursor, new_node.rule, head.depth - 1, new_node.nodeid});
	    //}
	  }
	}
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
	stack.insert(new_node);
	nodes.push_back(new_node);
	properties.push_back(properties[n]);
	parents.push_back(set<int>());
	ends.push_back(set<int>());
	crumbs.push_back(set<int>{n});

	heads.push(Head{new_node.cursor, new_node.rule, head.depth, new_node.nodeid});
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
	    //int n_ends = ends[id].size();
	    //if (!parents[id].count(n)) //should not be needed?
	    parents[id].insert(n);

	    //if already has ends, add nexts
	    set<int> ends_copy = ends[id];
	    for (int e : ends_copy) {
	      auto new_node = NodeIndex{e, r + 1, (int)nodes.size()};
	      if (stack.count(new_node)) {
		int existing_id = stack.find(new_node)->nodeid;
		int existing_prop = properties[existing_id];
		int our_prop = properties[n];
		if (existing_prop != our_prop) {
		  parents[existing_prop].insert(parents[our_prop].begin(), parents[our_prop].end());
		  properties[n] = properties[existing_prop];
		  auto new_node = nodes[existing_id];
		  heads.push(Head{new_node.cursor, new_node.rule, head.depth, new_node.nodeid});
		}
		
		continue; // add crumbs?
	      } else { //add node
		if (DEBUG) cout << "adding " << new_node << endl;
		stack.insert(new_node);
		nodes.push_back(new_node);
		properties.push_back(properties[n]);
		parents.push_back(set<int>{});
		ends.push_back(set<int>());
		crumbs.push_back(set<int>()); //TODO: how to get the return from the end?
		
		heads.push(Head{new_node.cursor, new_node.rule, head.depth, new_node.nodeid});
	      }
	    }
	  } else {
	    //create node
	    auto new_node = ni;

	    if (DEBUG) cout << "adding " << new_node << endl;
	    stack.insert(new_node);
	    nodes.push_back(new_node);
	    properties.push_back(new_node.nodeid);
	    parents.push_back(set<int>{n});
	    ends.push_back(set<int>());
	    crumbs.push_back(set<int>{n});
	    heads.push(Head{new_node.cursor, new_node.rule, head.depth + 1, new_node.nodeid});
	  }
	}
      }
      break;
    }
  }
}

#endif
