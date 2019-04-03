#include "gram.h"

const std::string gram_h_str = "S line \'\\n\' | line \'\\n\' S\n"\
"ws \'[ ]+\'\n"\
"line ruledef\n"\
"ruledef rulename ws options\n"\
"options option | options ws \'\\|\' ws option\n"\
"option ruleset\n"\
"ruleset rule ws ruleset | rule\n"\
"rule name | matchname | keyname\n"\
"keyname key \':\' name | key \':\' matchname\n"\
"key \'[_[:alpha:]]+\'\n"\
"rulename name\n"\
"matchname quote notquote quote | dquote notdquote dquote\n"\
"quote \"\\\'\"\n"\
"notquote \"[^\\\']*\"\n"\
"dquote \'\\\"\'\n"\
"notdquote \'[^\\\"]*\'\n"\
"name \'[_[:alpha:]]+\'\n";

std::string const gram2_h_str = "S line \'\\n\' | line \'\\n\' S\n"\
"ws \'[ ]+\'\n"\
"line ruledef | comment\n"\
"ruledef rulename ws options\n"\
"options option | options ws \'\\|\' ws option\n"\
"option ruleset\n"\
"ruleset rule ws ruleset | rule\n"\
"rule name | matchname | keyname\n"\
"keyname key \':\' name | key \':\' matchname\n"\
"key \'[_[:alpha:]]+\'\n"\
"rulename name\n"\
"matchname quote str:notquote quote | dquote str:notdquote dquote\n"\
"quote \"\\\'\"\n"\
"notquote \"[^\\\']*\"\n"\
"dquote \'\\\"\'\n"\
"notdquote \'[^\\\"]*\'\n"\
"name \'[_[:alpha:]]+\'\n"\
"comment \'#\' comstr:\'[^\\n]*\'\n";
