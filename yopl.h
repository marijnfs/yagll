#ifndef __YOPL_H__
#define __YOPL_H__

#include <vector>
#include <map>
#include <queue>
#include <re2/re2.h>
#include <string>
#include <algorithm>
#include <iterator>

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

struct Node {
  NodeIndex index;
  std::vector<int> return_list;
  std::map<int, std::vector<int>> parses;
};

struct Head {
  int cursor;
  int rule_ptr;
  int node_ptr;

  bool operator<(Head const &other) const {
    if (cursor != other.cursor)
      return cursor < other.cursor;
    return rule_ptr < other.rule_ptr;
  }

  bool operator>(Head const &other) const {
    if (cursor != other.cursor)
      return cursor > other.cursor;
    return rule_ptr > other.rule_ptr;
  }
};

enum RuleType {
  Option,
  Spawn,
  Match,
  End
};

struct RuleSetDef {
  std::vector<std::string> names;
  std::vector<RuleType> types;
  std::vector<std::string> args;

  void add_rule(std::string name, RuleType type, std::string arg = "") {
    names.push_back(name);
    types.push_back(type);
    args.push_back(arg);
  }
  
};

struct RuleSet {  
  std::vector<std::string> names;
  std::vector<RuleType> types;
  std::vector<std::vector<int>> arguments;
  std::vector<RE2*> matcher;

  RuleSet(RuleSetDef rulesetdef) {
    std::copy(rulesetdef.names.begin(), rulesetdef.names.end(), std::back_inserter(names));
     
  }
  
};

struct Graph {
  //rule stack
  std::vector<int> rules;
  std::vector<std::vector<int>> rule_top;
  
  //node stack
  std::vector<Node> nodes;
  std::vector<std::vector<int>> node_top;
  
};

#endif
