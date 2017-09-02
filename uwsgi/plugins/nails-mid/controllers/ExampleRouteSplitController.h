#ifndef _EXAMPLEROUTESPLITCONTROLLER_H_
#define _EXAMPLEROUTESPLITCONTROLLER_H_

#include "nails.h"
#include <stdio.h>

class ExampleRouteSplitController : public BaseController {
 public:
  nails_route<url("/custom/<id>"), methods("GET")>
  show(Request& req);

  Response foo(Request& req);
};

#endif
