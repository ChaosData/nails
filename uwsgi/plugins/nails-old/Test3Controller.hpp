#include <stdio.h>
#include "controller-helper.h"

class Test3Controller : public BaseController {
 public:
  void show(Request& req, Response& res) {
    (void)req;
    (void)res;
    puts("show3()");
  }

  void show2(Request req, Request* req2, Request** req3, Response& res, Response&& res2) {
    (void)req;
    (void)res;
    (void)req2;
    (void)res2;
    (void)req3;
    puts("showtwo3()");
  }

  void index(Request& req, Response& res) {
    (void)req;
    (void)res;
    puts("index3()");
  }

};


