#ifndef _GEN_H_
#define _GEN_H_
#include "controller-helper.h"

#include "TestController.h"
#include "Test2Controller.h"
#include "FooController.h"

extern "C" {
  void init_controllers();
  void teardown_controllers();
}


extern TestController singleton_TestController;
extern Test2Controller singleton_Test2Controller;
extern FooController singleton_FooController;

#endif
