#ifndef _TEST2CONTROLLER_H_
#define _TEST2CONTROLLER_H_

#include "controller-helper.h"

class Test2Controller : public BaseController, public func_base {
 public:
  Test2Controller();
  void show(Request& req, Response& res);
  void destroy(Request& req, Response& res);

};

#endif
