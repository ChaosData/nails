#include "controller-helper.h"
#include "gen.h"

int main() {
  init_controllers();

  dyncall(type_name(singleton_TestController), "index");
  dyncall(type_name(singleton_Test2Controller), "destroy");

  dyncall("Test", "index");
  dyncall("Test2", "destroy");
  dyncall("Test2", "index");
  dyncall("Test2", "show");

  teardown_controllers();
  return 0;
}

