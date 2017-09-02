#ifndef _GEN_H_
#define _GEN_H_
#include "controller-helper.h"

#include "../controllers/ExampleRouteSplitController.h"
#include "../controllers/FooController.h"
#include "../controllers/Test2Controller.h"
#include "../controllers/TestController.h"
#include "../controllers/ExampleRouteSingleController.hpp"
#include "../controllers/NewController.hpp"
#include "../controllers/Test3Controller.hpp"

extern "C" {
  void init_controllers();
  void teardown_controllers();
}

extern ExampleRouteSingleController singleton_ExampleRouteSingleController;
extern ExampleRouteSplitController singleton_ExampleRouteSplitController;
extern FooController singleton_FooController;
extern NewController singleton_NewController;
extern Test2Controller singleton_Test2Controller;
extern Test3Controller singleton_Test3Controller;
extern TestController singleton_TestController;

#endif