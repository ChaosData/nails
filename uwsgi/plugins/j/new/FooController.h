#ifndef _FOOCONTROLLER_H_
#define _FOOCONTROLLER_H_

#include "controller-helper.h"

class FooController : public BaseController, public func_base {
 public:
  FooController();

  void foo(Request& req, Response& res);
  void bar(Request& req, Response& res);
};


#endif
