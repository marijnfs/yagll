#include <iostream>
#include <vector>
#include <map>
#include <string>

using namespace std;

struct Rule {
  virtual int match(char *c, int size); // -1 not match, 0<= matched n
};

struct MatchCharRule : Rule {
  char m;

  int match(char *c, int size) {
    if (!size) return -1;
    if c[0] == m { return 1; }
    return -1;
  }
};

struct RuleRef {
  int rule, i;

  bool operator<(RuleRef &const other) const {
    if (rule == other.rule)
      return i < other.i;
    return rule < other.rule;
  }
};

struct GSS {
  map<RuleRef, set<RuleRef> > gss;
  set<RuleRef> popped;

  void add_link(RuleRef node, RuleRef parent);
  
};

vector<Rule> rules;
vector<int> rule_next;


int main(int argc, char **argv) {
  assert(argc == 2);

  string str(argv[1]);

  cout << "Starting" << endl;

  

}
