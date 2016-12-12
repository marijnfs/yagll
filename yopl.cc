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
#include <iostream>



using namespace std;

struct NodeIndex {
  int cursor, rule;
  bool operator<(NodeIndex const &other) const {
    if (cursor != other.cursor)
      return cursor < other.cursor;
    return rule < other.rule;
  }

  bool operator>(NodeIndex const &other) const {
    if (cursor != other.cursor)
      return cursor > other.cursor;
    return rule > other.rule;
  }
};

struct Head {
  int cursor, rule, depth;
  int node;
  
  bool operator<(Head const &other) const {
    if (cursor != other.cursor)
      return cursor < other.cursor;
    if (depth != other.depth)
      return depth < other.depth;
    return rule < other.rule;
  }

  bool operator>(Head const &other) const {
    if (cursor != other.cursor)
      return cursor > other.cursor;
    if (depth != other.depth)
      return depth > other.depth;
    return rule > other.rule;
  }
};

enum RuleType {
  OPTION = 0,
  PUSH = 1,
  MATCH = 2,
  RETURN = 3,
  END = 4
};

struct RuleSet {
  std::vector<std::string> names;
  std::vector<RuleType> types;
  std::vector<std::vector<int>> arguments;
  std::vector<RE2*> matcher;

  //RuleSet(RuleSetDef rulesetdef);

  void add_ret() {
    types.push_back(RETURN);
    arguments.push_back(vector<int>(0,0));
    matcher.push_back(0);
  }

  void add_end() {
    types.push_back(END);
    arguments.push_back(vector<int>(0,0));
    matcher.push_back(0);
  }

  void add_option(vector<int> spawn) {
    types.push_back(OPTION);
    arguments.push_back(spawn); //call S
    matcher.push_back(0);
  }

  void add_match(RE2 *rule) {
    types.push_back(MATCH);
    arguments.push_back(vector<int>(0,0));
    matcher.push_back(rule);
  }
  
};

int match(RE2 &matcher, string &str, int pos = 0) {
  re2::StringPiece match;
  if (matcher.Match(str, pos, str.size(), RE2::ANCHOR_START, &match, 1)) {
    cout << "Matched " << match.length() << endl;
    return match.length();
  }
  //failed
  return -1;
}

int main(int argc, char **argv) {
  set<NodeIndex> stack;
  vector<vector<int>> parents;
  vector<int> properties;
  
  priority_queue<Head> heads;

  string buffer("aabacaaa");
  RE2 rule("");

  cout << match(rule, buffer) << endl;
  return 1;
  /*
  int cursor = graph.cursors[head];
  re2::StringPiece match;
  if (reg.Match(buffer, cursor, buffer.size(), RE2::ANCHOR_START, &match, 1)) {
    cout << "Matched " << match.length() << endl;
    }*/
  
  //[Op3,END,-,S,a,-
  //[1  ,2  ,3,4,5

  
  RuleSet ruleset;
  ruleset.add_option(vector<int>{2});
  ruleset.add_end();
  ruleset.add_option(vector<int>{4,7});
  ruleset.add_ret();
  ruleset.add_option(vector<int>{2});
  ruleset.add_match(new RE2("a"));
  ruleset.add_ret();
  ruleset.add_match(new RE2(""));
  ruleset.add_ret();

  stack.insert(NodeIndex{0, 0});
  heads.push(Head{0, 0, 0, 0});

  while(heads.size()) {
    Head head = heads.top();
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
	vector<int> &par = parents[properties_node];
	for (int p : par) {
	  p;
	}
      }
      break;
    case MATCH:
      {
	int n = head.node;
	int cur = head.cursor;
	int m = match(*ruleset.matcher[n], buffer, cur);
      }
      break;
    case OPTION:
      {
	int n = head.node;
	int cur = head.cursor;
	vector<int> &args = ruleset.arguments[n];
	for (int r : args) {
	  NodeIndex ni{cur, r};
	  if (stack.count(ni)) {
	    int id = distance(stack.begin(), stack.find(ni));
	  }
	}
      }
      break;
    }
  }
}

#endif
