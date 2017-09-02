#include <string>
#include <stdio.h>


int main() {
  std::string hs = "hello:world";
  std::string f;
  f.assign(hs, 0, hs.find(':'));
  puts(f.c_str());
  return 0;
}
