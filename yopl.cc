#include "yopl.h"

#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char **argv) {\
  RuleSetDef rulesetdef;
  Graph graph;


  rulesetdef.add_rule("Base", End);
  rulesetdef.add_rule("S", Option, "S1 S2");
  rulesetdef.add_rule("S2", Spawn, "NP VP");
  rulesetdef.add_rule("S1", Spawn, "S PP");

  rulesetdef.add_rule("NP", Option, "NP1 NP2 NP3");
  rulesetdef.add_rule("NP1", Spawn, "*n");
  rulesetdef.add_rule("NP2", Spawn, "*det *n");
  rulesetdef.add_rule("NP3", Spawn, "NP PP");

  rulesetdef.add_rule("PP", Spawn, "*prep NP");
  rulesetdef.add_rule("VP", Spawn, "*v NP");

  rulesetdef.add_rule("*v", Match, "v");
  rulesetdef.add_rule("*n", Match, "n");
  rulesetdef.add_rule("*det", Match, "det");
  rulesetdef.add_rule("*prep", Match, "prep");
  

  RuleSet ruleset(rulesetdef);

  
}
