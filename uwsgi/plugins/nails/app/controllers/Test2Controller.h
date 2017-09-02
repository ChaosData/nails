#ifndef _TEST2CONTROLLER_H_
#define _TEST2CONTROLLER_H_

#include "nails.h"

class Test2Controller : public BaseController {
 public:
  Test2Controller();
  Response show(Request& req);
  Response destroy(Request& req);

};

#endif
