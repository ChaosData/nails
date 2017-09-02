#ifndef _GEN_H_
#define _GEN_H_
#include "controller-helper.h"

#include "../app/controllers/ExampleRouteSplitController.h"
#include "../app/controllers/FooController.h"
#include "../app/controllers/TestController.h"
#include "../app/controllers/Test2Controller.h"
#include "../app/controllers/Test3Controller.hpp"
#include "../app/controllers/ExampleRouteSingleController.hpp"
#include "../app/controllers/NewController.hpp"

extern "C" {
  void init_controllers();
  void teardown_controllers();
}

extern ChildExampleRouteSingleController singleton_ChildExampleRouteSingleController;
extern ExampleRouteSingleController singleton_ExampleRouteSingleController;
extern ExampleRouteSplitController singleton_ExampleRouteSplitController;
extern FooController singleton_FooController;
extern NewController singleton_NewController;
extern Test2Controller singleton_Test2Controller;
extern Test3Controller singleton_Test3Controller;
extern TestController singleton_TestController;

#endif