#ifndef _TESTCONTROLLER_H_
#define _TESTCONTROLLER_H_

#include "controller-helper.h"

class TestController : public BaseController, public func_base {
 public:
  TestController();

  void show(Request& req, Response& res);
  void show2(Request req, Request* req2, Request** req3, Response& res, Response&& res2);
  void index(Request& req, Response& res);
};

#endif
