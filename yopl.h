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


const bool DEBUG(false);

struct NodeIndex {
  int cursor = -1, rule = -1;
  int nodeid = -1;
  
  bool operator<(NodeIndex const &other) const;
  bool operator>(NodeIndex const &other) const;
};

std::ostream &operator<<(std::ostream &out, NodeIndex &ni);

enum RuleType {
  OPTION = 0,
  MATCH = 1,
  RETURN = 2,
  END = 3
};

std::ostream &operator<<(std::ostream &out, RuleType &t);

struct RuleSet {
  std::vector<std::string> names;
  std::vector<RuleType> types;
  std::vector<std::vector<int>> arguments;
  std::vector<RE2*> matcher;
  std::vector<int> returns; // points to return (or end) point of each rule, needed to place crumbs
  
  RuleSet();

  //parse a file with simple ruleset parser
  RuleSet(std::string filename);

  //add return op
  void add_ret();

  //add end file op
  void add_end();

  //add option
  void add_option(std::string name, std::vector<int> spawn = std::vector<int>());

  //add anonymous option
  void add_option(std::vector<int> spawn = std::vector<int>());

  //add RE2 string match
  void add_match(std::string name, std::string matchstr);

  //add anonymous RE2 string match
  void add_match(std::string matchstr);
  
  int size();
  
};

//Test a RE2 matcher on a string starting at pos 'pos'. return number of characters eaten, -1 for no match
int match(RE2 &matcher, std::string &str, int pos = 0);

struct Parser {
  RuleSet ruleset;

  // Parser keeps the whole parsing state here

  std::vector<NodeIndex> nodes; //all nodes are stored here
  std::vector<int> properties; //properties index, pointing of node containing shared info of nodes, such as:
  std::vector<std::set<int>> parents; //who is your parent? Can be multiple nodes that happen to spawn this rule at this cursor
  std::vector<std::set<int>> ends; //where does this rule end, info needed when spawning already ended nodes
  std::vector<std::set<int>> crumbs; //crumbs, for de-parsing
  std::set<NodeIndex> node_occurence; //occurence set, checking if a node already exists
  
  std::priority_queue<NodeIndex> heads; //active part, a queue with sorted nodes
  

  Parser(std::string gram_file);

  //add a node
  //does not check whether it exists
  void add_node(NodeIndex node, int prop_node, int parent = -1, int crumb = -1); 

  //parse a file
  int parse(std::string input_file);
};

#endif
