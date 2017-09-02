#ifndef _GEN_H_
#define _GEN_H_
#include "controller-helper.h"

#include "TestController.h"
#include "Test2Controller.h"
#include "FooController.h"
#include "ExampleRouteSplitController.h"
#include "ExampleRouteSingleController.hpp"

extern "C" {
  void init_controllers();
  void teardown_controllers();
}

extern ExampleRouteSingleController singleton_ExampleRouteSingleController;
extern ExampleRouteSplitController singleton_ExampleRouteSplitController;
extern FooController singleton_FooController;
extern Test2Controller singleton_Test2Controller;
extern TestController singleton_TestController;

#endif