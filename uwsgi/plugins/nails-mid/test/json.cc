#include "json.hpp"
#include <stdio.h>

using json = nlohmann::json;


int main(int argc, char** argv) {
  auto j = json::parse(argv[argc-1]);
  printf("j.empty(): %u\n", j.empty());
  return 0;
}
