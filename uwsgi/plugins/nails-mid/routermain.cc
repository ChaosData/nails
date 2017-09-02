#include "router.h"
#include "gen.h"

int main(int argc, char** argv) {
  init_controllers();

  func_base* fun = (func_base*)1;

  router.add(argv[1], {"GET"}, fun, type_name(singleton_ExampleRouteSingleController));

  #ifdef PPROFFING
  HeapProfilerStart("prefixx5");
  #endif
  for (int i(2); i<argc; i++) {
    std::unordered_map<std::string,std::string> route_params;
    auto ca = router.match(argv[i], "GET", route_params);
    if (ca.second != nullptr) {
      for (auto const& kv : route_params) {
        printf("first: %s\n", kv.first.c_str());
        printf("second: %s\n", kv.second.c_str());
      }
    } else {
      puts("Did not match.");
      for (auto const& kv : route_params) {
        printf("first: %s\n", kv.first.c_str());
        printf("second: %s\n", kv.second.c_str());
      }
    }
    puts("===================");
  }
  //HeapProfilerDump("prefixx");
  //HeapProfilerStop();

  teardown_controllers();
  return 0;
}

