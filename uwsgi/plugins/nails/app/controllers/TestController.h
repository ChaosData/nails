#ifndef _TESTCONTROLLER_H_
#define _TESTCONTROLLER_H_

#include "nails.h"

class TestController : public BaseController {
 public:
  TestController();

  Response show(Request& req);
  void show2(Request req, Request* req2, Request** req3, Response& res, Response&& res2);
  Response index(Request& req);
};

#endif
