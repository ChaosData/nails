#include <regex>
#include <string>
#include <iostream>
#include <stdio.h>


int main() {
  std::string fnames[] = {"gg", "^ggggg", "# hello", "#hello"};
  std::regex base_regex{"(\\^|#)([^\\}]*)"};
  
// Extraction of a sub-match
  std::smatch base_match;
 
  for (const auto &fname : fnames) {
    if (std::regex_match(fname, base_match, base_regex)) {
      // The first sub_match is the whole string; the next
      // sub_match is the first parenthesized expression.
      if (base_match.size() == 3) {
        std::ssub_match base_sub_match = base_match[1];
        std::string base = base_sub_match.str();
        std::cout << fname << " has a base of " << base << '\n';
      }
    }
  }
  return 0;
}
