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
#include "parser.h"

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
  return out << "c" << ni.cursor << " r" << ni.rule << " i" << ni.id << " l" << ni.level;
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

Parser::Parser(string gram_file, LoadType load_type)
    : ruleset(gram_file, load_type) {
  cout << "created parser for gram: " << gram_file << " load type: " << load_type << endl;
}

void Parser::push_node(int cursor, int rule, int prop_node, int parent,
                       int crumb, int level) { // does not check whether it exists
  NodeIndex node{cursor, rule, int(nodes.size()), level};

  node_occurence.insert(node);
  nodes.push_back(node);

  properties.push_back(prop_node == -1 ? node.id : prop_node);
  parents.push_back(parent == -1 ? set<int>() : set<int>{parent});
  crumbs.push_back(crumb == -1 ? set<int>() : set<int>{crumb});
  ends.push_back(set<int>());
  returned.push_back(false);
  
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
  end_node = -1; // index to the end node, only gets set by the END rule
  while (heads.size()) {
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
    int l = head.level;
      
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
      push_node(cur + m, r + 1, properties[id], -1, id, l);
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
          push_node(cur, new_r, -1, id, id, l + 1);
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
              push_node(end_node.cursor, r + 1, properties[id], -1, e, l + 1);
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
        returned[p] = true;

        //auto new_node = NodeIndex{cur, nodes[p].rule + 1};
        
        // if (!node_occurence.count(new_node)) {// ||
        // ruleset.types[new_node.rule] == RETURN) {
        push_node(cur, nodes[p].rule + 1, properties[p], -1, id, l - 1);
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

void Parser::fail_message() {
    cout << "FAILED" << endl;
    int line_start(0), line_end(0);
    int line(0);
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

  //// post processing
  vector<int> active_nodes;
    priority_queue<int> q;
  q.push(end_node);
  // int n = end_node;

  // Pass one: follow the crumbs
  // follow crumbs from end to beginning
  // figure out which nodes were actually part of the successful parse
  vector<bool> active(nodes.size());
  cout << "nodes.size: " << nodes.size() << endl;

  while (!q.empty()) {
    int n = q.top();
    q.pop();

    if (active[n])
      continue;

    //active_nodes.push_back(n);
    active[n] = true;

    //active_nodes.push_back(n); ///to remove
    for (int c : crumbs[n])
      q.push(c);
  }
  
  //Pass two, follow children
  //Needed, since the crumbs might still contain parts of failed parse tries

  //Make children links
  vector<set<int>> children(nodes.size());
  for (int n(0); n < nodes.size(); ++n)
    for (int p : parents[properties[n]]) {
      if (n == properties[n])
        cout << ruleset.names[nodes[p].rule] << " " << p << " " << n << endl;
      children[p].insert(n);
    }
  std::priority_queue<int, std::vector<int>, std::greater<int> > q2;
  q2.push(0);
  
  vector<bool> active_pass2(size());
  vector<bool> seen_nodes(size());
  
  while (!q2.empty()) {
    int n = q2.top();
    q2.pop();
    
    if (seen_nodes[n])
      continue;
    seen_nodes[n] = true;
    

    if (active[n] && (!children[n].size() || returned[n])) { //nodes with children (options) need to have returned, this filters out the wrong paths
      active_pass2[n] = true;
      for (int c : children[n])
        q2.push(c);
    }
  }
  
  // Pass three
  // Same as first but only to make sure whe know where every node ends
  
  vector<int> ends(size());
  q.push(end_node);
  
  while (q.size()) {
    int n = q.top();
    // cout << "q: " << n << endl;
    q.pop();
    if (!active[n]) { //reusing active vector to prevent looping
      cout << "repeat: " << ruleset.names[nodes[n].rule] << " " << nodes[n].cursor << endl;
      continue;
    }
    active[n] = false; 

    if (active_pass2[n])
      active_nodes.push_back(n);

    int c = nodes[n].cursor;
    for (int crumb : crumbs[n]) {
      ends[crumb] = c;
      q.push(crumb);
    }
  }
  reverse(active_nodes.begin(), active_nodes.end());

  /*
    for (auto a : active_nodes) {
    cout << a << " c:" << nodes[a].cursor << " r:" << nodes[a].rule << " " << returned[a] << " ch:";
    for (auto c : children[a])
      cout << c << " ";
    cout << endl;
  }
  
  cout << "n active:" << active_nodes.size() << endl;
  */

  //reverse(active_nodes.begin(), active_nodes.end());

  map<int, int> node_map;
  int n_nodes(0);
  for (int n : active_nodes)
    node_map[n] = n_nodes++;

  // map<int, int> last_with_parent; // map containing original node id for node
  // who last had this parent, needed so
  // siblings can determine ends of last sibling
  auto &rname_map = pg.rname_map;
  auto &name_map = pg.name_map;

  for (int n : active_nodes) {
    NodeIndex &node = nodes[n];
    int new_node_id = node_map[n];

    pg.nodes.push_back(ParsedNode{new_node_id});
    pg.starts.push_back(node.cursor);
    pg.ends.push_back(ends[n]);
    pg.cleanup.push_back(false);
    pg.levels.push_back(node.level);
    
    ParsedNode &pn(last(pg.nodes));

    // get name id from rule with name
    int name_id(-1);
    auto rule_name = ruleset.names[node.rule];
    // if (!rule_name.size() && ruleset.matcher[node.rule]) //give matcher
    // pattern as name
    //  rule_name = ruleset.matcher[node.rule]->pattern();
    if (rule_name.size()) {
      if (!rname_map.count(rule_name)) {
        name_id = rname_map.size();
        rname_map[rule_name] = name_id;
        name_map[name_id] = rule_name;
      } else
        name_id = rname_map[rule_name];
    }

    pg.name_ids.push_back(name_id);
  }
  
  for (int n : active_nodes) {
    int new_node_id = node_map[n];
    ParsedNode &pn(pg.nodes[new_node_id]);
    
    // make link from parent to child
    for (auto p : parents[properties[n]]) {
      if (!node_map.count(p)) // parent is not in node_map yet, so not active
        continue;
      int new_p = node_map[p];

      pg.nodes[new_p].children.insert(new_node_id);
      pg.nodes[new_node_id].parents.insert(new_p);
    }
  }

  for (auto n : pg.nodes) //leafs should have the right ends, not propagate this to parents in a no-brainer way
    for (auto p : n.parents)
      pg.ends[p] = max(pg.ends[p], pg.starts[n.n]);

  // we have to end the leaf node.
  // We run through them in sequence, and in theory by construction they should
  // be consecutive, so we can end them with the start of the next leaf
  // Possible error: In ambiguous parse cases both cases might be represented, this screws things up
  
  /*int last_n = -1;
  for (auto n : pg.nodes)
    if (!n.children.size()) {
      if (last_n != -1)
        pg.ends[last_n] = pg.starts[n.n];
      last_n = n.n;
    }*/

  // cout << "leaf: " << pg.name_map[pg.name_ids[n.n]] << " " << pg.starts[n.n]
  // << endl;

  return unique_ptr<ParseGraph>(parse_graph_ptr);
}

void Parser::dot_graph_debug(string filename) {
  ofstream dotfile(filename);

  dotfile << "digraph parsetree {" << endl;

  ostringstream oss_rank;
  /*oss_rank << "{rank = same; ";
  for (int i(0); i < buffer.size(); ++i) {
    if (buffer[i] == '\n' || buffer[i] == ' ' || buffer[i] == '\\')
      continue;
    dotfile << "char" << i << " [label=\"" << buffer[i] << "\"]" << endl;
    oss_rank << "char" << i << "; ";
  }
  oss_rank << "}";
  dotfile << oss_rank.str() << endl;*/

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

void Parser::dot_graph_final(string filename) {
  ofstream dotfile(filename);

  dotfile << "digraph parsetree {" << endl;

  dotfile << "}" << endl;
}

unique_ptr<ParseGraph> Parser::parse(string input_file) {
  cout << "parsing " << input_file << endl;
  load(input_file);
  process();
  dot_graph_debug("debug.dot");
  
  auto pg = post_process();
  if (!pg)
    return pg;

  pg->print_dot("post-process-debug.dot");
  
  // basic whitespace and no-name filter
  pg->filter([](ParseGraph &pg, int n) {
    if (pg.name_ids[n] == -1)
      pg.cleanup[n] = true;
    if (pg.name_map[pg.name_ids[n]] == "ws")
      pg.cleanup[n] = true;
  });
  pg->compact();
  return pg;
}

void Parser::reset() {
  nodes.clear();
  properties.clear();
  parents.clear();
  ends.clear();
  crumbs.clear();
  node_occurence.clear();
  returned.clear();
  heads = priority_queue<NodeIndex>(); // for whatever reason pqueue doesn't have clear

  end_node = 0;
  furthest = 0;
}
