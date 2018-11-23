#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <queue>
#include <re2/re2.h>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "const.h"
#include "yagll.h"

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
    return out << "OPTION";
    break;
  case MATCH:
    return out << "MATCH";
    break;
  case RETURN:
    return out << "RETURN";
    break;
  case END:
    return out << "END";
    break;
  }
}

int match(RE2 &matcher, string &str, int pos) {
  re2::StringPiece match;
  if (matcher.Match(str, pos, str.size(), RE2::ANCHOR_START, &match, 1)) {
    if (DEBUG)
      cout << "Matched " << match.length() << endl;
    return match.length();
  }
  // failed
  return -1;
}

Parser::Parser(istream &infile, LoadType load_type)
    : ruleset(infile, load_type) {
  cout << "created parser for gram, load type: " << load_type << endl;
}

void Parser::push_node(int cursor, int rule, int prop_node, int parent,
                       int crumb, int prev) { // does not check whether it exists
  NodeIndex node{cursor, rule, int(nodes.size()), prev};

  node_occurence.insert(node);
  nodes.push_back(node);

  properties.push_back(prop_node == -1 ? node.id : prop_node);
  parents.push_back(parent == -1 ? set<int>() : set<int>{parent});
  crumbs.push_back(crumb == -1 ? set<int>() : set<int>{crumb});
  ends.push_back(set<int>());
  prevs.push_back(prev);
  
  heads.push(node);
  if (DEBUG)
    cout << "adding: c" << cursor << " r" << rule << " i" << node.id << " p"
         << prop_node << " pa" << parent << " c" << crumb << endl;
}

void Parser::load(istream &infile) {
  buffer =
      string(istreambuf_iterator<char>(infile), istreambuf_iterator<char>());
}

void Parser::process() {
  heads = priority_queue<NodeIndex>(); // clear your head! //stupid pqueue
                                       // doesn't have clear method

  // add the ROOT node
  push_node(0, 0, 0); // cursor, rule, prop node

  // Start Parsing
  end_node = -1; // index to the end node, only gets set by the END rule
  while (heads.size()) {
    NodeIndex head = heads.top(); // pop head of the queue
    heads.pop();

    furthest = max(furthest, head.cursor);

    if (DEBUG)
      cout << "head: " << head << " " << ruleset.types[head.rule] << endl;

    // convenience
    int id = head.id;
    int cur = head.cursor;
    int r = head.rule;
    int prev = head.prev;

    //cout << cur << " " << id << " " << r << " " << ruleset.types[r] << endl;
    //if (ruleset.names[r].size())
    //  cout << ruleset.names[r] << endl;
    
    switch (ruleset.types[r]) {
    case END:
      if (cur == buffer.size()) {
        end_node = id;
        continue;
      }
      break;

    case MATCH: {
      // Matcher: advanced to the next rule and puts cursor after the match
      // options node will be same as current node
      // crumb will be this node
      int m = match(*ruleset.matcher[r], buffer, cur);
      if (DEBUG)
        cout << "Match rule: '" << ruleset.matcher[r]->pattern() << "' [" << cur
             << "] matched " << m << endl;

      if (m < 0)
        break; // no match

      // all is well, add the node
      push_node(cur + m, r + 1, properties[id], -1, -1, id);
    } break;
    case OPTION: {
      // Option: Spawn one or more child rules which (possibly) return back and
      // then this rule ends and the next rule starts
      vector<int> &args = ruleset.arguments[r]; // spawn rules are stored here

      for (int new_r : args) {
        NodeIndex ni{cur, new_r};

        // A node with same rule at this cursor can already be created by
        // another rule (not rarely recursively, especially with right-recursive
        // rules!) We check if the node exists first
        if (!node_occurence.count(ni)) {
          // create new node
          push_node(cur, new_r, -1, id, -1, -1);
        } else {
          // node already exists, get the node
          int child_id = node_occurence.find(ni)->id;

          parents[child_id].insert(id); // add current node as a parent as well
          //crumbs[child_id].insert(id);  // we are the crumb now, also

          // if already has ends, spawn nodes with the next rule
          // this gets complicated as crumbs have to be set properly as well

          set<int> ends_copy = ends[child_id];
          for (int e : ends_copy) {
            // we can spawn the next node
            auto end_node = nodes[e];
            int end_c = end_node.cursor;
            auto new_node = NodeIndex{end_c, r + 1};
            if (!node_occurence.count(new_node))
              push_node(end_node.cursor, r + 1, properties[id], -1, e, id);
            else {
              int id = node_occurence.find(new_node)->id;
              ends[id].insert(e);
              
            }
          }
        }
      }
    } break;
    case RETURN: {
      int properties_node = properties[id];
      ends[properties_node].insert(id);
      
      set<int> par = parents[properties_node];

      for (int p : par)
        push_node(cur, nodes[p].rule + 1, properties[p], -1, id, p);
      
    } break;
    }
    if (end_node >= 0)
      break;
  }
}

void Parser::fail_message() {
    cout << "FAILED" << endl;
    int line_start(0), line_end(0);
    int line(1);
    int cur(0);
    for (int i(0); i <= buffer.size(); ++i) {
      line_end = i;
      if (i < furthest) {
        if (buffer[i] == '\n') {
          line_start = i;
          ++line;
          cur = 0;
        } else
          ++cur;
      } else {
        if (buffer[i] == '\n')
          break;
      }
    }

    cout << "at line: " << line << ":" << cur << endl;

    auto end_string = buffer.substr(line_start, line_end - line_start);
    replace(end_string.begin(), end_string.end(), '\t', ' ');
    cout << "got to: " << endl << end_string << endl;
    cout << string(cur, ' ') << "^" << endl;

    for (auto n : nodes) {
      if (n.cursor == furthest && ruleset.types[n.rule] == MATCH)
        cout << "trying to match: " << ruleset.matcher[n.rule]->pattern()
             << endl;
    }
}

unique_ptr<ParseGraph> Parser::post_process() {
  ParseGraph *parse_graph_ptr = new ParseGraph();
  ParseGraph &pg(*parse_graph_ptr);

  pg.buffer = buffer;

  if (end_node < 0) { /// Deal with missed parse, give some indication why it
                   /// failed
    fail_message();
    return unique_ptr<ParseGraph>(nullptr);
  }

  /// new postprocess approach
  struct QNode {
    int p = 0, n = 0;
  };

  //ParseGraph pg;
  pg.buffer = buffer;
  queue<QNode> q;
  q.push(QNode{0, end_node});
  pg.add_node(0, 0, buffer.size(), "BASE");

  while (q.size()) {
    QNode node = q.front();
    int n = node.n;
    int p = node.p;
    q.pop();
      
    int r = nodes[n].rule;
    int c = nodes[n].cursor;
    RuleType rt = ruleset.types[r];
      
    vector<int> node_sequence;
    int prev = n;
    while (prev != -1) {
      node_sequence.push_back(prev);
      prev = prevs[prev];
    }
    reverse(node_sequence.begin(), node_sequence.end());

    for (int i(1); i < node_sequence.size(); ++i) {
      int prev = node_sequence[i-1];
      int current = node_sequence[i];
        
      int new_r = nodes[prev].rule;
      int start_c = nodes[prev].cursor;
      int end_c = nodes[current].cursor;

      string rulename = ruleset.names[new_r];
      int nodeid = pg.size();
        
      pg.add_node(nodeid, start_c, end_c, rulename);
      pg.add_connection(p, nodeid);

      for (int c : crumbs[current]) {
        q.push(QNode{nodeid, c});
      }
        
      //q.push(prev);
      //current = prev;
      //last_c = new_c;        
    }
  }

  // basic whitespace and no-name filter
  pg.filter([](ParseGraph &pg, int n) {
      if (pg.name_ids[n] == -1)
        pg.cleanup[n] = true;
      if (pg.name_map[pg.name_ids[n]] == "ws")
        pg.cleanup[n] = true;
    });

  //static int bloe = 0;
  //ostringstream oss;
    
  //oss << "bloe_" << bloe++ << ".dot";
  //pg.print_dot(oss.str());
  return unique_ptr<ParseGraph>(parse_graph_ptr);
}

void Parser::dot_graph_debug(string filename) {
  ofstream dotfile(filename);

  dotfile << "digraph parsetree {" << endl;

  for (int i(0); i < nodes.size(); ++i) {
    dotfile << "node" << i << " [shape=\"box\", label=\""
            << ruleset.names[nodes[i].rule] << " "
            << ruleset.types[nodes[i].rule] << " " << nodes[i] << "\"]" << endl;
    //dotfile << "node" << i << " -> char" << nodes[i].cursor
    //<< " [color=\"grey\"]" << endl;
    for (auto c : crumbs[i])
      dotfile << "node" << i << " -> node" << c << " [style=dashed, color=grey]"
              << endl;

    // if (properties[i] == i)
    //for (auto p : parents[properties[i]])
    //dotfile << "node" << i << " -> node" << p
    //        << " [color=\"black:invis:black\"]" << endl;
  }
  dotfile << "}" << endl;
}

unique_ptr<ParseGraph> Parser::parse(istream &infile) {
  load(infile);
  process();
  
  auto pg = post_process();
  if (!pg)
    return pg;

  // basic whitespace and no-name filter
  pg->filter([](ParseGraph &pg, int n) {
    if (pg.name_ids[n] == -1)
      pg.cleanup[n] = true;
    if (pg.name_map[pg.name_ids[n]] == "ws")
      pg.cleanup[n] = true;
  });
  return pg;
}

void Parser::reset() {
  nodes.clear();
  properties.clear();
  parents.clear();
  ends.clear();
  crumbs.clear();
  node_occurence.clear();
  heads = priority_queue<NodeIndex>(); // for whatever reason pqueue doesn't have clear

  end_node = 0;
  furthest = 0;
}
