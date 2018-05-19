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

Parser::Parser(string gram_file) : ruleset(gram_file) {}

void Parser::push_node(int cursor, int rule, int prop_node, int parent,
                       int crumb) { // does not check whether it exists
  NodeIndex node{cursor, rule, int(nodes.size())};

  node_occurence.insert(node);
  nodes.push_back(node);

  properties.push_back(prop_node == -1 ? node.id : prop_node);
  parents.push_back(parent == -1 ? set<int>() : set<int>{parent});
  crumbs.push_back(crumb == -1 ? set<int>() : set<int>{crumb});
  ends.push_back(set<int>());

  heads.push(node);
  if (DEBUG)
    cout << "adding: c" << cursor << " r" << rule << " i" << node.id << " p"
         << prop_node << " pa" << parent << " c" << crumb << endl;
}

void Parser::load(string filename) {
  ifstream infile(filename);
  buffer =
      string(istreambuf_iterator<char>(infile), istreambuf_iterator<char>());
}

void Parser::process() {
  heads = priority_queue<NodeIndex>(); // clear your head! //stupid pqueue
                                       // doesn't have clear method

  // add the ROOT node
  push_node(0, 0, 0); // cursor, rule, prop node

  // Start Parsing
  end_node = 0; // index to the end node, only gets set by the END rule
  while (heads.size() && end_node == 0) {
    NodeIndex head = heads.top(); // pop head of the queue
    heads.pop();

    furthest = max(furthest, head.cursor);

    if (DEBUG)
      cout << "head: " << head << " " << ruleset.types[head.rule] << endl;

    /*if (nodes.size() > 1000) {
      cout << "nodes: ";
      for (int n(0); n < nodes.size(); ++n) {
      cout << nodes[n].cursor << "r" << nodes[n].rule << " ";
      }
      cout << endl;
      return -1;
      }*/

    // what type of rule is this node

    // convenience
    int id = head.id;
    int cur = head.cursor;
    int r = head.rule;

    switch (ruleset.types[r]) {
    case END:
      cout << "END " << cur << " " << buffer.size() << endl;
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
      push_node(cur + m, r + 1, properties[id], -1, id);
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
          push_node(cur, new_r, -1, id, id);
        } else {
          // node already exists, get the node
          int child_id = node_occurence.find(ni)->id;

          parents[child_id].insert(id); // add current node as a parent as well
          crumbs[child_id].insert(id);  // we are the crumb now, also

          // if already has ends, spawn nodes with the next rule
          // this gets complicated as crumbs have to be set properly as well

          set<int> ends_copy = ends[child_id];
          for (int e : ends_copy) {
            // we can spawn the next node
            auto end_node = nodes[e];
            auto new_node = NodeIndex{end_node.cursor, r + 1};

            if (!node_occurence.count(
                    new_node)) { // TODO: maybe we should actually keep checking
                                 // for ends here
              // add node
              push_node(end_node.cursor, r + 1, properties[id], -1, e);
            } else {
              int existing_id = node_occurence.find(new_node)->id;
              // int existing_prop = properties[existing_id];
              // int our_prop = properties[n];

              // parents[existing_prop].insert(parents[our_prop].begin(),
              // parents[our_prop].end());
              crumbs[existing_id].insert(e);
            }
          }
        }
      }
    } break;
    case RETURN: {
      int properties_node = properties[id];
      ends[properties_node].insert(id);

      set<int> par = parents[properties_node];

      for (int p : par) {
        auto new_node = NodeIndex{cur, nodes[p].rule + 1};

        // if (!node_occurence.count(new_node)) {// ||
        // ruleset.types[new_node.rule] == RETURN) {
        push_node(cur, nodes[p].rule + 1, properties[p], -1, id);
        //} else {
        // int existing_id = node_occurence.find(new_node)->id;
        // cout << "exist, add to " << existing_id << " " << nodes[existing_id]
        // << endl; crumbs[existing_id].insert(head.id);
        // }
      }
    } break;
    }
  }
}

unique_ptr<ParseGraph> Parser::post_process() {
  ParseGraph *parse_graph_ptr = new ParseGraph();
  ParseGraph &pg(*parse_graph_ptr);

  //// post processing
  vector<int> active_nodes;
  if (!end_node) {
    cout << "FAILED" << endl;
    cout << "at char: " << furthest << endl;
    auto end_string = buffer.substr(max(0, furthest - 5), 10);
    replace(end_string.begin(), end_string.end(), '\n', ' ');
    cout << "got to: " << endl << end_string << endl;
    cout << "     ^" << endl;

    for (auto n : nodes) {
      if (n.cursor == furthest && ruleset.types[n.rule] == MATCH)
        cout << "trying to match: " << ruleset.matcher[n.rule]->pattern()
             << endl;
    }
    return unique_ptr<ParseGraph>(nullptr);
  }

  set<int> seen_nodes;
  queue<int> q;
  q.push(end_node);
  // int n = end_node;

  // follow crumbs from end to beginning
  // figure out which nodes were actually part of the successful parse
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

  // run through active nodes, filtering and ending
  set<string> filter_set;
  filter_set.insert("ws"); // filter whitespace

  vector<set<int>> children;

  map<int, int> node_map;
  map<int, int> last_with_parent; // map containing original node id for node
                                  // who last had this parent, needed so siblings
                                  // can determine ends of last sibling
  auto &name_map = pg.name_map;
  auto &reverse_name_map = pg.reverse_name_map;

  int n_parse_nodes(0);

  for (int n : active_nodes) {
    int new_node_id = n_parse_nodes++;

    NodeIndex &node = nodes[n];
    pg.nodes.push_back(ParsedNode{new_node_id});
    ParsedNode &pn(last(pg.nodes));
    // cout << "active: " << node << endl;

    node_map[n] =
        new_node_id; // node_map converts from old node id to new node id

    auto rule_name = ruleset.names[node.rule];
    int name_id(-1);
    if (rule_name.size()) {
      if (!name_map.count(rule_name)) {
        name_id = name_map.size();
        name_map[rule_name] = name_id;
        reverse_name_map[name_id] = rule_name;
      } else
        name_id = name_map[rule_name];
    }

    pg.starts.push_back(node.cursor);
    pg.ends.push_back(-1);
    pg.name_ids.push_back(name_id);
    pg.cleanup.push_back(false);
    
    // make link from parent to child
    for (auto p : parents[properties[node.id]]) {
      if (!node_map.count(p)) // parent is not in node_map yet, so not active
        continue;
      int new_p = node_map[p]; // we assume parent is already encountered, since
                               // we move forward over active nodes
      if (last_with_parent.count(
              new_p)) // someone had this parent, must be sibling, end him
        pg.ends[last_with_parent[new_p]] = node.cursor;
      last_with_parent[new_p] = new_node_id;
      pg.nodes[new_p].children.insert(new_node_id);
      pn.parents.insert(new_p);
    }
  }
  
  
  return unique_ptr<ParseGraph>(parse_graph_ptr);
}

void Parser::dot_graph_debug(string filename) {
  ofstream dotfile(filename);

  dotfile << "digraph parsetree {" << endl;

  ostringstream oss_rank;
  oss_rank << "{rank = same; ";
  for (int i(0); i < buffer.size(); ++i) {
    if (buffer[i] == '\n' || buffer[i] == ' ' || buffer[i] == '\\')
      continue;
    dotfile << "char" << i << " [label=\"" << buffer[i] << "\"]" << endl;
    oss_rank << "char" << i << "; ";
  }
  oss_rank << "}";
  dotfile << oss_rank.str() << endl;

  for (int i(0); i < nodes.size(); ++i) {
    dotfile << "node" << i << " [shape=\"box\", label=\""
            << ruleset.names[nodes[i].rule] << " "
            << ruleset.types[nodes[i].rule] << " " << nodes[i] << "\"]" << endl;
    dotfile << "node" << i << " -> char" << nodes[i].cursor
            << " [color=\"grey\"]" << endl;
    for (auto c : crumbs[i])
      dotfile << "node" << i << " -> node" << c << " [style=dashed, color=grey]"
              << endl;

    // if (properties[i] == i)
    for (auto p : parents[properties[i]])
      dotfile << "node" << i << " -> node" << p
              << " [color=\"black:invis:black\"]" << endl;
  }
  dotfile << "}" << endl;
}

void Parser::dot_graph_final(string filename) {
  ofstream dotfile(filename);

  dotfile << "digraph parsetree {" << endl;

  dotfile << "}" << endl;
}

unique_ptr<ParseGraph> Parser::parse(string input_file) {
  load(input_file);
  process();
  dot_graph_debug("debug.dot");
  return post_process();
}
