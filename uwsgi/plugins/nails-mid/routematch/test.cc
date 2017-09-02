#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <regex>
#include <stdio.h>


/*
  /foo/<action>
  /foo/bar/<action>
  /foo/bar/GG<action>
  /foo/bar/<action>/2

=>

  "foo" -> end
  -> "bar" -> end
     -> "GG" -> <action> -> end
  -> <action> -> end
     -> "2" -> end

  notes:
  - most specific "wins"
  - need to use a tree-like structure to match things
    - initialization can be bootstrapped during nails-preroute phase
  - should have capability for spcial literals to be included in string match
    - need to be careful w/ this if only b/c uwsgi might give me arbitrary
      representations for certain chars depending on whether or not they were
      sent url-encoded in the request, which might then be browser specific
    - e.g. quotes, <, >
      - literal / is "pre-handled" :P
  - should be able to handle static chars in token and multiple variables in a
    segment
*/


std::vector<std::string>&
split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::vector<std::string>
split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, elems);
  return elems;
}

auto segmentize(std::string const& path) {
  if (path.size() == 0 || path[0] != '/') {
    return std::vector<std::string>();
  }
  auto ret = split(path, '/');
  ret.erase(ret.begin());
  return ret;
}


int main() {
  std::vector<std::string> tokens = segmentize("/testing123/foo//bar");
  for (auto const& s : tokens) {
    puts(s.c_str());
  }

  return 0;
}
