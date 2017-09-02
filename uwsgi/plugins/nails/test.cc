#include <string>
#include <stdio.h>


void trim(std::string& s) {
  size_t start = s.find_first_not_of(' ');
  size_t end = s.find_last_not_of(' ');
  s.assign(s, start, end+1 - start);
}

int main() {
  std::string s = "  hello  world     ";
  printf("s(%lu): >%s<\n", s.size(), s.c_str());
  trim(s);
  printf("s(%lu): >%s<\n", s.size(), s.c_str());

  return 0;
}
