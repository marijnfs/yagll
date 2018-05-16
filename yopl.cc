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

#include "yopl.h"

using namespace std;

bool NodeIndex::operator<(NodeIndex const &other) const {
  if (cursor != other.cursor)
    return cursor < other.cursor;
  return rule > other.rule;
}

bool NodeIndex::operator>(NodeIndex const &other) const {
  if (cursor != other.cursor)
    return cursor > other.cursor;
  return rule < other.rule;
}

ostream &operator<<(ostream &out, NodeIndex &ni) {
  return out << "c" << ni.cursor << " r" << ni.rule << " i" << ni.id;
}

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


int match(RE2 &matcher, string &str, int pos) {
  re2::StringPiece match;
  if (matcher.Match(str, pos, str.size(), RE2::ANCHOR_START, &match, 1)) {
    if (DEBUG) cout << "Matched " << match.length() << endl;
    return match.length();
  }
  //failed
  return -1;
}

Parser::Parser(string gram_file) : ruleset(gram_file) {
}

void Parser::push_node(int cursor, int rule, int prop_node, int parent, int crumb) { //does not check whether it exists
  NodeIndex node{cursor, rule, int(nodes.size())};

  node_occurence.insert(node);
  nodes.push_back(node);

  properties.push_back(prop_node == -1 ? node.id : prop_node);
  parents.push_back(parent == -1 ? set<int>() : set<int>{parent});
  crumbs.push_back(crumb == -1 ? set<int>() : set<int>{crumb});
  ends.push_back(set<int>());

  heads.push(node);
  if (DEBUG)
    cout << "adding: c" << cursor << " r" << rule << " i" << node.id << " p" << prop_node << " pa" << parent  << " c" << crumb << endl;

}

void Parser::load(string filename) {
  ifstream infile(filename);
  buffer = string(istreambuf_iterator<char>(infile),
                     istreambuf_iterator<char>());
}

void Parser::process() {
  heads = priority_queue<NodeIndex>(); //clear your head! //stupid pqueue doesn't have clear method
  
  //add the ROOT node
  push_node(0, 0, 0); //cursor, rule, prop node
  
  //Start Parsing
  end_node = 0; //index to the end node, only gets set by the END rule
  while(heads.size() && end_node == 0) {
    NodeIndex head = heads.top(); //pop head of the queue
    heads.pop();
    
    cout << "POP: " << head.cursor << " " << head.rule << " " << heads.size() <<  endl;
    
    furthest = max(furthest, head.cursor);
    
    if (DEBUG) cout << "head: " << head << " " << ruleset.types[head.rule] << endl;
    
    /*if (nodes.size() > 1000) {
      cout << "nodes: ";
      for (int n(0); n < nodes.size(); ++n) {
      cout << nodes[n].cursor << "r" << nodes[n].rule << " ";
      }
      cout << endl;
      return -1;
      }*/
    
    // what type of rule is this node

    //convenience
    int id = head.id;
    int cur = head.cursor;
    int r = head.rule;
    cout << "prop: " << properties[id];
    if (parents[properties[id]].size()) {
      cout << " parent:" ;
      for (auto p : parents[properties[id]])
        cout << p << " ";
    }
    cout << endl;
      
    switch (ruleset.types[r]) {
    case END:
      cout << "END " << cur << " " << buffer.size() << endl;
      if (cur == buffer.size()) {
        end_node = id;
        continue;
      }
      break;
      
    case MATCH:
      {
        //Matcher: advanced to the next rule and puts cursor after the match
        //options node will be same as current node
        //crumb will be this node
        int m = match(*ruleset.matcher[r], buffer, cur);
        if (DEBUG)
          cout << "Match rule: '" << ruleset.matcher[r]->pattern() << "' [" << cur<< "] matched " << m << endl;
        
        if (m < 0)
          break; //no match
        
        //all is well, add the node
        push_node(cur + m, r + 1, properties[id], -1, id);
      }
      break;
    case OPTION:
      {
        //Option: Spawn one or more child rules which (possibly) return back and then this rule ends and the next rule starts
        vector<int> &args = ruleset.arguments[r]; //spawn rules are stored here
        
        for (int new_r : args) {
          NodeIndex ni{cur, new_r, (int)nodes.size()};

          //A node with same rule at this cursor can already be created by another rule (not rarely recursively, especially with right-recursive rules!)
          //We check if the node exists first
           if (!node_occurence.count(ni)) {
            //create new node
             push_node(cur, new_r, -1, id, id);
          } else {
            //node already exists, get the node
            int id = node_occurence.find(ni)->id;
            
            parents[id].insert(id); //add current node as a parent as well
            crumbs[id].insert(id); //we are the crumb now, also
            
            //if already has ends, spawn nodes with the next rule
            //this gets complicated as crumbs have to be set properly as well
            
            set<int> ends_copy = ends[id];
            for (int e : ends_copy) {
              //we can spawn the next node
              auto end_node = nodes[e];
              auto new_node = NodeIndex{end_node.cursor, r + 1};
              
              if (!node_occurence.count(new_node)) { //TODO: maybe we should actually keep checking for ends here
                //add node
                push_node(end_node.cursor, r + 1, properties[id], -1, e);
              } else {
                int existing_id = node_occurence.find(new_node)->id;
                //int existing_prop = properties[existing_id];
                //int our_prop = properties[n];
                
                //parents[existing_prop].insert(parents[our_prop].begin(), parents[our_prop].end());
                crumbs[existing_id].insert(e);
              }
            }
          }
        }
      }
      break;
    case RETURN:
      {
        int properties_node = properties[id];
        ends[properties_node].insert(id);
        
        set<int> par = parents[properties_node];
        cout << "p: " << properties_node << " par: " << parents[properties_node].size() << endl;
        for (int p : par) {
          auto new_node = NodeIndex{cur, nodes[p].rule + 1};
	  
          if (!node_occurence.count(new_node)) {// || ruleset.types[new_node.rule] == RETURN) {
            push_node(cur, nodes[p].rule + 1, properties[p], -1, id);
          } else {
            int existing_id = node_occurence.find(new_node)->id;
            //int existing_prop = properties[existing_id];
            //int parent_prop = properties[p];
	      
            //parents[existing_prop].insert(parents[parent_prop].begin(), parents[parent_prop].end());
            crumbs[existing_id].insert(head.id);
          }
        }
      }
      break;
    }
  }
}

int Parser::post_process() {
  //// post processing
  vector<int> active_nodes;
  if (!end_node) {
    cout << "FAILED" << endl;
    cout << "at char: " << furthest << endl;
    cout << "got to: " << buffer.substr(max(0, furthest - 5), 10) << endl;
    return 1;
  }

  
  set<int> seen_nodes;
  queue<int> q;
  q.push(end_node);
  //int n = end_node;

  while (!q.empty()) {
    int n = q.front();
    if (!seen_nodes.count(n)) {
      active_nodes.push_back(n);
      seen_nodes.insert(n);
	  
      for (int c : crumbs[n])
        q.push(c);
    }
    q.pop();
  }

  reverse(active_nodes.begin(), active_nodes.end());


  //run through active nodes, filtering and ending
  set<string> filter_set;
  filter_set.insert("ws");

  vector<string> names;
  vector<int> starts;
  vector<int> ends;
  vector<set<int>> children;

  map<int, int> node_map;
  int n_parse_nodes(0);

  bool last_was_match(false); int last_n(0); //little hacky, matches dont return
  for (int n : active_nodes) {
    NodeIndex &node = nodes[n];
    cout << "active: " << node << endl;

    if (ruleset.names[node.rule].size() && !filter_set.count(ruleset.names[node.rule])) {
      node_map[n] = n_parse_nodes++;
      names.push_back(ruleset.names[node.rule]);
      starts.push_back(node.cursor);
      ends.push_back(-1);
      children.push_back(set<int>());
	  
      //make link from parent to child
      for (int p : parents[properties[n]])
        if (node_map.count(properties[p]))
          children[node_map[properties[p]]].insert(node_map[n]);
    }

	
    //set end for the matching rule of return
    if (ruleset.types[node.rule] == RETURN)
      ends[node_map[properties[n]]] = node.cursor;
    //for (int p : parents[properties[n]])
    //if (node_map.count(p))
    //ends[node_map[p]] = node.cursor;

    //set ends for MATCH nodes
    if (last_was_match) {
      ends[node_map[last_n]] = node.cursor;
      last_was_match = false;
    }
    if (ruleset.types[node.rule] == MATCH) {
      last_was_match = true;
      last_n = n;
    }

	
    /*
      cout << "node: " << n << " cursor: " << nodes[n].cursor << " "  << ruleset.types[nodes[n].rule] << endl;
      //int p = properties[n];
      if (parents[n].size()) {
      cout << "parents: ";
      for (auto p : parents[n])
      cout << p << " ";
      cout << endl;
      }
    */
  }

  ofstream dotfile("parsetree.dot");

  dotfile << "digraph parsetree {" << endl;      
  for (int i(0); i < n_parse_nodes; ++i) {
    if (ends[i] >= 0) {//not all active nodes are valid; if they are unended they never matched completely
      cout << names[i] << " " << starts[i] << "-" << ends[i] << " [" << buffer.substr(starts[i],  ends[i] - starts[i]) << "]" << endl;
      dotfile << "node_" << i << " [label=\"" << names[i] << " [" << buffer.substr(starts[i],  ends[i] - starts[i]) << "]\"];" << endl;
      for (int c : children[i]) {
        if (ends[c] != -1)
          dotfile << "node_" << i << " -> node_" << c << ";" << endl;
      }
    }
  }
  dotfile << "}" << endl;
  cout << "SUCCESS" << endl;
  return 0;
}

int Parser::parse(string input_file) {
  load(input_file);
  process();
  return post_process();
}

