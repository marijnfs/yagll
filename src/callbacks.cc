#include "parsegraph.h"

using namespace std;

TypeGraphCallback::TypeGraphCallback(ParseGraph *pg_) : pg(pg_) {
}

void TypeGraphCallback::register_callback(string type, CallbackFunc func) {
  callbacks[type] = func;
  types_set.insert(type);
}

void TypeGraphCallback::operator()(int n) {
  auto t = pg->type(n);
  callbacks[t](n);
}

bool TypeGraphCallback::match(int n) {
  return types_set.count(pg->type(n));
}

bool TypeGraphCallback::add_children(int n) {
  return types_set.count(pg->type(n)) == 0;
}

void BottomUpGraphCallback::run_default(int n) {
}

void BottomUpGraphCallback::operator()(int n) {
  auto t = pg->type(n);
  if (types_set.count(t) != 0)
    return run_default(n);
  callbacks[t](n);
}

bool BottomUpGraphCallback::match(int n) {
  return true;
}

bool BottomUpGraphCallback::add_children(int n) {
  return true;
}
