#include "ExampleRouteSingleController.hpp"
#include "controller-helper.h"

int main() {

  ExampleRouteSingleController e;
  Request r{};
  Response r2{};
  e.fooer(r,r2);
  return 0;
}
