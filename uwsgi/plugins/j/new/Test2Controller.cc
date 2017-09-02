#include <stdio.h>
#include "controller-helper.h"
#include "Test2Controller.h"

Test2Controller::Test2Controller() { }

void Test2Controller::show(Request& req, Response& res) {
  (void)req;
  (void)res;
  puts("2:show()");
}

void Test2Controller::destroy(Request& req, Response& res) {
  (void)req;
  (void)res;
  puts("2:destroy()");
}
