#ifndef _GEN_H_
#define _GEN_H_

#include "controller.h"

void init_controllers();
void teardown_controllers();

extern TestController singleton_TestController;
extern Test2Controller singleton_Test2Controller;


#endif
