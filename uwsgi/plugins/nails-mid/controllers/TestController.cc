#include <stdio.h>
#include "nails.h"
#include "TestController.h"

TestController::TestController() { }

Response TestController::show(Request& req) {
  (void)req;
  puts("show()");
  return "show()";
}

void TestController::show2(Request req, Request* req2, Request** req3, Response& res, Response&& res2) {
  (void)req;
  (void)res;
  (void)req2;
  (void)res2;
  (void)req3;
  puts("show()");
}

Response TestController::index(Request& req) {
  (void)req;
  puts("index()");
  puts("index()");
  return "dobule index()";
}
