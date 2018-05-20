#ifndef __RULESET_H__
#define __RULESET_H__

#include <iostream>
#include <string>
#include <vector>
#include <re2/re2.h>

enum RuleType { OPTION = 0, MATCH = 1, RETURN = 2, END = 3 };

std::ostream &operator<<(std::ostream &out, RuleType &t);

struct RuleSet {
  std::vector<std::string> names;
  std::vector<RuleType> types;
  std::vector<std::vector<int>> arguments;
  std::vector<RE2 *> matcher;
  std::vector<int> returns; // points to return (or end) point of each rule,
                            // needed to place crumbs

  RuleSet();

  // parse a file with simple ruleset parser
  RuleSet(std::string filename);

  // add return op
  void add_ret();

  // add end file op
  void add_end();

  // add option
  void add_option(std::string name,
                  std::vector<int> spawn = std::vector<int>());

  // add anonymous option
  void add_option(std::vector<int> spawn = std::vector<int>());

  // add RE2 string match
  void add_match(std::string name, std::string matchstr);

  // add anonymous RE2 string match
  void add_match(std::string matchstr);

  int size();
};

// Test a RE2 matcher on a string starting at pos 'pos'. return number of
// characters eaten, -1 for no match
int match(RE2 &matcher, std::string &str, int pos = 0);


#endif
