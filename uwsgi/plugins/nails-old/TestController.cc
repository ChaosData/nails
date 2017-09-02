#include <stdio.h>
#include "controller-helper.h"
#include "TestController.h"

TestController::TestController() { }

void TestController::show(Request& req, Response& res) {
  (void)req;
  (void)res;
  puts("show()");
}

void TestController::show2(Request req, Request* req2, Request** req3, Response& res, Response&& res2) {
  (void)req;
  (void)res;
  (void)req2;
  (void)res2;
  (void)req3;
  puts("show()");
}

void TestController::index(Request& req, Response& res) {
  (void)req;
  (void)res;
  puts("index()");
  puts("index()");
}
