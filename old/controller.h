#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "controller-helper.h"

class TestController : public BaseController, public func_base {
 public:
  TestController();

  void show(Request& req, Response& res);
  void show2(Request req, Request* req2, Request** req3, Response& res, Response&& res2);
  void index(Request& req, Response& res);

  private:
   //ControllerActionMapper& mapper_;
};

class Test2Controller : public BaseController, public func_base {
 public:
  Test2Controller();
  void show(Request& req, Response& res);
  void destroy(Request& req, Response& res);

 private:
  //ControllerActionMapper& mapper_;
};

#endif
