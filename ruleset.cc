#include "yopl.h"

using namespace std;

RuleSet::RuleSet() {
}
  
RuleSet::RuleSet(string filename) {
  ifstream infile(filename.c_str());
  
  enum Mode {
    BLANK = 0,
    READ = 1,
    ESCAPESINGLE = 2,
    ESCAPEDOUBLE = 3
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
          mode = ESCAPESINGLE;
        else if (c == '\"')
          mode = ESCAPEDOUBLE;
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
      else if (mode == ESCAPESINGLE) {
        if (c == '\'') {
          //add item to set
          curitems.push_back(item);
          item.clear();
          mode = BLANK;
        } else
          item += c;
      }
      else if (mode == ESCAPEDOUBLE) {
        if (c == '\"') {
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
  add_option("ROOT", vector<int>{2}); //TODO should not be hardcoded
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

void RuleSet::add_ret() {
  names.push_back("");
  types.push_back(RETURN);
  arguments.push_back(vector<int>(0,0));
  matcher.push_back(0);
  returns.push_back(0);
}

void RuleSet::add_end() {
  names.push_back("");
  types.push_back(END);
  arguments.push_back(vector<int>(0,0));
  matcher.push_back(0);
  returns.push_back(0);
}

void RuleSet::add_option(string name, vector<int> spawn) {
  names.push_back(name);
  types.push_back(OPTION);
  arguments.push_back(spawn); //call S
  matcher.push_back(0);
  returns.push_back(0);
}

void RuleSet::add_option(vector<int> spawn) {
  add_option("", spawn);
}

void RuleSet::add_match(string name, string matchstr) {
  names.push_back(name);
  types.push_back(MATCH);
  arguments.push_back(vector<int>(0,0));
  matcher.push_back(new RE2(matchstr));
  returns.push_back(0);
}

void RuleSet::add_match(string matchstr) {
  add_match("", matchstr);
}

int RuleSet::size() {
  return types.size();
}
