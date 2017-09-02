#ifndef _EXAMPLEROUTESPLITCONTROLLER_H_
#define _EXAMPLEROUTESPLITCONTROLLER_H_

#include "controller-helper.h"
#include <stdio.h>

class ExampleRouteSplitController : public BaseController {
 public:
  nails_route<url("/custom/:id"), methods("GET")>
  show(Request& req, Response& res);

  void foo(Request& req, Response& res);
};

#endif
