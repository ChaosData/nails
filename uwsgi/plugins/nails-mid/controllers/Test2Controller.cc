#include <stdio.h>
#include "nails.h"
#include "Test2Controller.h"

Test2Controller::Test2Controller() { }

Response Test2Controller::show(Request& req) {
  (void)req;
  puts("2:show()");
  return "2:show()";
}

Response Test2Controller::destroy(Request& req) {
  (void)req;
  puts("2:destroy()");
  return "2:destroy()";
}
