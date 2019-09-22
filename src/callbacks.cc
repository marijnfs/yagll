#include "parsegraph.h"

using namespace std;

TopDownCallback::TopDownCallback(ParseGraph *pg_) : pg(pg_) {
}

void TopDownCallback::register_callback(string type, CallbackFunc func) {
  callbacks[type] = func;
  types_set.insert(type);
}

void TopDownCallback::operator()(int n) {
  auto t = pg->type(n);
  callbacks[t](n);
}

bool TopDownCallback::match(int n) {
  return types_set.count(pg->type(n));
}

bool TopDownCallback::add_children(int n) {
  return types_set.count(pg->type(n)) == 0;
}

void BottomUpCallback::run_default(int n) {
}

void BottomUpCallback::operator()(int n) {
  auto t = pg->type(n);
  if (types_set.count(t) != 0)
    return run_default(n);
  callbacks[t](n);
}

bool BottomUpCallback::match(int n) {
  return true;
}

bool BottomUpCallback::add_children(int n) {
  return true;
}
