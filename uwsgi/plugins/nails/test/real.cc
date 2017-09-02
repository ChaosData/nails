#include <limits.h>
#include <stdlib.h>
#include <stdio.h>


int main(int argc, char** argv) {
  char const* real = realpath(argv[1], nullptr);
  puts(real);
  free((void*)real);
  return 0;
}
