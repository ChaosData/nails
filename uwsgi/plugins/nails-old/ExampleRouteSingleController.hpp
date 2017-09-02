#ifndef _EXAMPLEROUTESPLITCONTROLLER_HPP_
#define _EXAMPLEROUTESPLITCONTROLLER_HPP_

#include "controller-helper.h"
#include <stdio.h>

class ExampleRouteSingleController : public BaseController {
 public:

  //template<Url url, Methods methods>
  nails_route<url("/custom/:id/destroy"), methods("DELETE", "POST")>
  destroyer(Request& req, Response& res) {
    (void)req;
    (void)res;
    puts("destroy");
  };

  void fooer(Request& req, Response& res) {
    (void)req;
    (void)res;
    puts("fooer");
  }

  void fooer2(Request& req, Response& res) {
    (void)req;
    (void)res;
    puts("fooer2");
  }

 private:
  void fooer3(Request& req, Response& res) {
    (void)req;
    (void)res;
    puts("fooer3");
  }

};

#endif
