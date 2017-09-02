#include <string>
#include <stdio.h>

class Foo {
 public:
  explicit Foo(void) = default;

  Foo(std::string const& s) : str(s) { }
  Foo(char const* cs) : str(cs) { }



  std::string str;
};

Foo getFoo() {
  return "gg";
}

int main() {
  puts(getFoo().str.c_str());
  return 0;
}
