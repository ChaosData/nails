#include <stdio.h>
#include <string>

bool startsWith(std::string const& str, std::string const& head) {
  size_t strL = str.size(), headL = head.size();
  if (strL >= headL) {
    return 0 == str.compare(0, headL, head);
  }
  return false;
}

int main(int argc, char** argv) {
  if (startsWith(argv[1], argv[2])) {
    puts("startsWith!");
  } else {
    puts("nope.");
  }
  return 0;
}
