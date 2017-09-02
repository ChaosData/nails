#include "FooController.h"

FooController::FooController() { }

void FooController::foo(Request& req, Response& res) {
  (void)req;
  (void)res;
  puts("foo");
}

void FooController::bar(Request& req, Response& res) {
  (void)req;
  (void)res;
  puts("bar");
}
