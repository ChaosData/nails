#ifndef _EXAMPLEROUTESPLITCONTROLLER_HPP_
#define _EXAMPLEROUTESPLITCONTROLLER_HPP_

#include <stdio.h>
#include <type_traits>

class Request { };
class Response { };

class ExampleRouteSingleController {
 public:

  template<int a>
  std::enable_if_t<a==5>
  destroyer(Request& req, Response& res) {
    (void)req;
    (void)res;
    puts("destroy");
  }

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

};

#endif
