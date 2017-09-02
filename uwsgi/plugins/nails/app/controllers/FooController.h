#ifndef _FOOCONTROLLER_H_
#define _FOOCONTROLLER_H_

#include "nails.h"

class FooController : public BaseController {
 public:
  FooController();

  Response foo(Request& req);
  Response bar(Request& req);
};


#endif
