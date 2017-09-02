#include <stdio.h>
#include "nails.h"

class Test3Controller : public BaseController {
 public:
  Response show(Request& req) {
    (void)req;
    puts("show3()");
    return "show3()";
  }

  void show2(Request req, Request* req2, Request** req3, Response& res, Response&& res2) {
    (void)req;
    (void)res;
    (void)req2;
    (void)res2;
    (void)req3;
    puts("showtwo3()");
  }

  Response index(Request& req) {
    (void)req;
    puts("index3()");
    return "index3()";
  }

};


