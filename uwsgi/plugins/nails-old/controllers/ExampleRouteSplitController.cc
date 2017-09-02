#include "ExampleRouteSplitController.h"

nails_route<url("/custom/:id"), methods("GET")>
ExampleRouteSplitController::show(Request& req, Response& res) {
  (void)req;
  (void)res;
  puts("show");
}
nails_route_explicit(ExampleRouteSplitController::show);

void ExampleRouteSplitController::foo(Request& req, Response& res) {
  (void)req;
  (void)res;
  puts("foo");
}

