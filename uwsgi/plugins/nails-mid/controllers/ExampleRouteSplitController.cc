#include "ExampleRouteSplitController.h"

nails_route<url("/custom/<id>"), methods("GET")>
ExampleRouteSplitController::show(Request& req) {
  (void)req;
  puts("show");
  return "show";
}
nails_route_explicit(ExampleRouteSplitController::show);

Response ExampleRouteSplitController::foo(Request& req) {
  (void)req;
  puts("foo");
  return "foo";
}

