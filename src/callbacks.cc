#include "parsegraph.h"

using namespace std;



TypeCallback::TypeCallback(ParseGraph *pg_, Mode mode_) : pg(pg_), mode(mode_) {
}

void TypeCallback::register_callback(string type, CallbackFunc func) {
  callbacks[type] = func;
  types_set.insert(type);
}

void TypeCallback::operator()(int n) {
  auto t = pg->type(n);
  if (mode == BOTTOM_UP && types_set.count(t) != 0)
    return run_default(n);
  callbacks[t](n);
}

bool TypeCallback::match(int n) {
  return mode == BOTTOM_UP || types_set.count(pg->type(n));
}

bool TypeCallback::add_children(int n) {
  return mode == BOTTOM_UP || types_set.count(pg->type(n)) == 0;
}

void TypeCallback::run_default(int n) {
}
