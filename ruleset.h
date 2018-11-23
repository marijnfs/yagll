#ifndef __RULESET_H__
#define __RULESET_H__

#include <iostream>
#include <re2/re2.h>
#include <string>
#include <vector>

enum RuleType { OPTION = 0, MATCH = 1, RETURN = 2, END = 3 };
enum LoadType { LOAD_BASIC = 0, LOAD_YOPL = 1, LOAD_YOPLYOPL = 2 };

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
  RuleSet(std::istream &infile, LoadType load_type = LOAD_YOPLYOPL);

  void basic_load(std::istream &infile);
  void yopl_load(std::istream &infile, LoadType load_type);
  void load(std::istream &infile, LoadType load_type);

  // add return op
  int add_ret();

  // add end file op
  int add_end();

  // add option
  int add_option(std::string name, std::vector<int> spawn = std::vector<int>());

  // add anonymous option
  int add_option(std::vector<int> spawn = std::vector<int>());

  // add RE2 string match
  int add_match(std::string name, std::string matchstr);

  // add anonymous RE2 string match
  int add_match(std::string matchstr);

  int size();
};

// Test a RE2 matcher on a string starting at pos 'pos'. return number of
// characters eaten, -1 for no match
int match(RE2 &matcher, std::string &str, int pos = 0);

#endif
