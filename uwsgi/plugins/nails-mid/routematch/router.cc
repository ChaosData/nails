#include <string>
#include <sstream>
#include <regex>
#include <unordered_map>
#include <stdexcept>
#include <utility>
#include <memory>

#include "controller-helper.h"

#include <google/heap-profiler.h>




std::vector<std::string>
split(std::string const& original, char separator) {
  size_t n = std::count(original.begin(), original.end(), separator);

  std::vector<std::string> results;
  results.reserve(n+2);
  std::string::const_iterator start = original.begin();
  std::string::const_iterator end = original.end();
  std::string::const_iterator next = std::find( start, end, separator );
  while ( next != end ) {
    results.push_back( std::string( start, next ) );
    start = next + 1;
    next = std::find( start, end, separator );
  }
  results.push_back( std::string( start, next ) );
  return results;
}







using TypeMatchFunc = bool (*) (std::string const&);

bool stringMatcher(std::string const&) {
  return true;
}

std::regex intMatcher_regex("[0-9]+",
    std::regex::ECMAScript|std::regex::optimize);
bool intMatcher(std::string const& path_segment) {
  return std::regex_match(path_segment, intMatcher_regex);
}

std::regex floatMatcher_regex("[0-9]+(?:\\.[0-9]+|)",
    std::regex::ECMAScript|std::regex::optimize);
bool floatMatcher(std::string const& path_segment) {
  return std::regex_match(path_segment, floatMatcher_regex);
}

std::unordered_map<std::string,TypeMatchFunc> type_matchers{
  {"", &stringMatcher},
  {"string", &stringMatcher},
  {"int", &intMatcher},
  {"float", &floatMatcher}
};


class RouteNode {
//mvp naive implementation
//hold entire route structure for all routes

 public:
  RouteNode() : parent(*this), segment_str("") { }

  //goal is to recursively add routes
  bool addRoute(std::string const& path, func_base* func);
  func_base* match(std::string const& path,
                   std::unordered_map<std::string,
                                      std::string>& route_params) const;

  enum Type {
    UNINIT,
    TEXT,
    SIMPLE,
    COMPLEX
  };

// private:
                                  //type      , name
  using FormatVariable = std::pair<std::string,std::string>;
 private:
  void addRoute(std::vector<std::string> const& segments,
                std::vector<std::string>::const_iterator cseg_iter,
                func_base* func);
  func_base* match(std::vector<std::string> const& segments,
                   std::vector<std::string>::const_iterator cseg_iter,
                   std::unordered_map<std::string,
                                      std::string>& route_params) const;

  static bool containsParameters(std::string const& segment);
  static bool isValidVariable(std::string const& variable);
  static bool isValid(std::string const& segment);
  static bool isSimpleVariable(std::string const& variable);

  bool matchesTEXT(std::string const& path_segment) const;
  bool matchesSIMPLE(std::string const& path_segment) const;
  bool matchesCOMPLEX(std::string const& path_segment,
                      std::unordered_map<std::string,
                                         std::string>& route_params) const;

 public:
  void extractSimpleVariableRoute(std::string const& segment);
  void extractComplexFormatRoute(std::string const& segment);

  func_base* matched_func = nullptr;
  Type type = UNINIT;
  std::reference_wrapper<RouteNode> parent;

  std::string segment_str;

  FormatVariable simpleVariable;
  std::regex complexFormatRegex;

  std::string complexFormatRegexStr;
  std::vector<FormatVariable> complexregrexFormatVariables;
  //regex will do initial match on chars
  //then apply format variable checks in order on capture groups

  std::unordered_map<std::string, RouteNode> children;
};

bool RouteNode::addRoute(std::string const& path, func_base* func) {
  std::vector<std::string> segments = split(path, '/');

  //validate entire path
  for (auto const& seg : segments) {
    if (!isValid(seg)) {
      return false;
    }
  }

  addRoute(segments, segments.cbegin(), func);
  return true;
}


void RouteNode::addRoute(std::vector<std::string> const& segments,
                         std::vector<std::string>::const_iterator cseg_iter,
                         func_base* func) {
  if (cseg_iter == segments.cend()) {
    return;
  }
  std::string const& cseg = *cseg_iter;
  segment_str = cseg;

  if (!containsParameters(cseg)) {
    type = TEXT;
  } else {
    if (isSimpleVariable(cseg)) {
    // SIMPLE
      extractSimpleVariableRoute(cseg);
      type = SIMPLE;
    } else {
      // COMPLEX
      // /_id-<int:id>-version-<int:version>_/
      extractComplexFormatRoute(cseg);
      type = COMPLEX;
    }
  }

  if (cseg_iter+1 != segments.end()) {
    std::string const& child_cseg = *(cseg_iter+1);
    auto const& child_iter_search = children.find(child_cseg);
    std::reference_wrapper<RouteNode> child = children.begin()->second;
    if (child_iter_search == children.end()) {
      child = children[child_cseg];
      child.get().parent = *this;
    } else {
      child = children[child_cseg];
    }
    child.get().addRoute(segments, cseg_iter+1, func);
  } else {
    if (matched_func != nullptr) {
      std::string fullpath;
      for (auto const& seg : segments) {
        fullpath += seg;
      }
      //note: this is obvious
      fprintf(stderr, "Duplicate route occurred. Overwriting route for %s.\n",
              fullpath.c_str());
      //note: /foo/<string:bar>/baz1 and /foo/<string:notbar>/baz2 are not
      //      and will likely lead to one or both of /foo/xxx/baz1 and
      //      /foo/xxx/baz2 to fail to match either
      //      - the best that can be done here is to validate for this after
      //        and wail about it then
      //        - nope, can't do that. it's valid. this needs to work
      //           - wait. i'm an idiot. it will work b/c a failed match will
      //             move onto/backtrack to the next child/choice to keep
      //             matching. they shouldn't be sharing intermediate nodes
    }
    matched_func = func;
  }
}



func_base* RouteNode::match(std::string const& path,
    std::unordered_map<std::string,std::string>& route_params) const {

  std::vector<std::string> segments = split(path, '/');

  return match(segments, segments.cbegin(), route_params);
}

func_base* RouteNode::match(std::vector<std::string> const& segments,
    std::vector<std::string>::const_iterator cseg_iter,
    std::unordered_map<std::string,std::string>& route_params) const {

  std::string const& cseg = *cseg_iter;

  printf("path segment: %s\n", cseg_iter->c_str());
  printf("route segment: %s\n", segment_str.c_str());
  switch (type) {
    case TEXT: {
      if (!matchesTEXT(cseg)) {
        return nullptr;
      }
      break;
    }
    case SIMPLE: {
      if (!matchesSIMPLE(cseg)) {
        return nullptr;
      }
      break;
    }
    case COMPLEX: {
      if (!matchesCOMPLEX(cseg, route_params)) {
        return nullptr;
      }
      break;
    }
    case UNINIT: {
      std::string fullpath;
      for (auto const& seg : segments) {
        fullpath += "/" + seg;
      }
      fullpath = fullpath.substr(1);
      std::reference_wrapper<RouteNode> p = parent;
      std::string fullroute = segment_str;
      while (std::addressof(p.get()) != std::addressof(p.get().parent.get())) {
        fullroute = p.get().segment_str + "/" + fullroute;
        p = p.get().parent;
      }
      //note: this is obvious
      fprintf(stderr, "UNINIT route matched.\n"
                      "\tReceived path: %s\n"
                      "\tMatched route: %s\n",
              fullpath.c_str(),
              fullroute.c_str());
      return nullptr;
    }
  }

  //match children if further path segments exist
  //store return value temporarily (if not nullptr) to extract params
  func_base* ret = nullptr;
  if (cseg_iter+1 != segments.end()) {
    bool found = false;
    for (auto const& kv : children) {
      ret = kv.second.match(segments, cseg_iter+1, route_params);
      if (ret != nullptr) {
        found = true;
        break;
      }
    }
  } else {
    ret = matched_func; //will be nullptr if this is not a matchable node
  }

  //extract (or remove) route params
  switch (type) {
    case UNINIT: {
      //this is here to shut the compiler up
      return nullptr;
    }
   case TEXT: {
      break;
    }
    case SIMPLE: {
      if (ret != nullptr) {
        route_params[simpleVariable.second] = cseg;
      }
      break;
    }
    case COMPLEX: {
      if (ret == nullptr) {
        for (auto const& p : complexregrexFormatVariables) {
          route_params.erase(p.second);
        }
      }
      break;
    }
  }
  return ret;
}

bool RouteNode::containsParameters(std::string const& segment) {
//assumes segment is already known to be valid
  if (segment.size() == 0) {
    return false;
  } else if (segment[0] == '<') {
    return true;
  }
  for (auto c = segment.cbegin()+1; c != segment.cend(); ++c) {
    if (*c == '<' && c[-1] != '\\') {
      return true;
    }
  }
  return false;
}

bool RouteNode::isValid(std::string const& segment) {
  if (segment.size() == 0) {
    return true;
  }

  bool open = false;
  char const* vbegin = nullptr;
  char const* vend = nullptr;

  if (segment[0] == '<') {
    open = true;
    vbegin = &(segment[0]);
  }

  for (auto c = segment.cbegin()+1; c < segment.cend(); ++c) {
    if (*c == '<') {
      if (open) {
        return false;
      }
      if (c[-1] != '\\') {
        open = true;
        if (c+1 == segment.cend()) {
          return false;
        }
        vbegin = &(*c);
      }
    } else if (*c == '>') {
      if (!open && c[-1] != '\\') {
        return false;
      }
      vend = &(*c);
      open = false;

      if (vbegin+1 == vend) {
        return false;
      }
      std::string variable{vbegin+1,vend-1};

      if (!isValidVariable(variable)) {
        return false;
      }
      vbegin = nullptr;
      vend = nullptr;
    }
  }
  if (vbegin != nullptr || vend != nullptr) {
    return false;
  }
  return true;
}

std::regex const isValidVariable_regex("(?:[a-zA-Z0-9_.]+:)?[a-zA-Z0-9_.]+",
    std::regex::ECMAScript|std::regex::optimize);
bool RouteNode::isValidVariable(std::string const& variable) {
  return std::regex_match(variable, isValidVariable_regex);
}

std::regex isSimpleVariable_regex("<(?:[a-zA-Z0-9_.]+:)?[a-zA-Z0-9_.]+>",
    std::regex::ECMAScript|std::regex::optimize);
bool RouteNode::isSimpleVariable(std::string const& variable) {
  return std::regex_match(variable, isSimpleVariable_regex);
}

std::regex extractSimpleVariableRoute_regex("<(?:([a-zA-Z0-9_.]+):)?([a-zA-Z0-9_.]+)>",
    std::regex::ECMAScript|std::regex::optimize);
void RouteNode::extractSimpleVariableRoute(std::string const& segment) {
  std::cmatch match;
  if (std::regex_match(segment.c_str(), match, extractSimpleVariableRoute_regex)) {
    if (match.size() == 2) {
      simpleVariable = std::make_pair(
                         "",
                         std::string(segment.data() + match.position(1),
                                     match[1].length()
                         )
                       );
    } else if (match.size() == 3) {
      simpleVariable = std::make_pair(
                         std::string(segment.data() + match.position(1),
                                     match[1].length()
                         ),
                         std::string(segment.data() + match.position(2),
                                     match[2].length()
                         )
                       );
    } else {
      simpleVariable = std::make_pair("<error>","<error>"); //illegal values
    }
  }
}


bool isRegexMetacharacter(char c) {
  switch (c) {
    case '\\': {
      return true;
      break;
    }
    case '^': {
      return true;
      break;
    }
    case '$': {
      return true;
      break;
    }
    case '*': {
      return true;
      break;
    }
    case '+': {
      return true;
      break;
    }
    case '?': {
      return true;
      break;
    }
    case '|': {
      return true;
      break;
    }
    case '(': {
      return true;
      break;
    }
    case ')': {
      return true;
      break;
    }
    case '[': {
      return true;
      break;
    }
    case ']': {
      return true;
      break;
    }
    case '.': {
      return true;
      break;
    }
    case '{': {
      return true;
      break;
    }
    case '}': {
      return true;
      break;
    }
    default: {
      return false;
    }
  }
}

void RouteNode::extractComplexFormatRoute(std::string const& segment) {
  //init performance

  bool open = false;
  char const *tbegin, *tend, *vbegin, *vcolon, *vend = nullptr;

  std::string& ss = complexFormatRegexStr;
  if (segment.size() > 15) {
    ss.reserve(segment.size() + 16);
    // hedging on most of the length being in the variable names
    //it'll lag behind stringstream for shorter/more likely segments in
    //space/allocation efficiency, though it should be much faster 
  }
  if (segment[0] == '<') {
    open = true;
    vbegin = &(segment[0]);
  } else {
    tbegin = &(segment[0]);
  }

  for (auto c = segment.cbegin()+1; c < segment.cend(); ++c) {
    if (*c == '<') {
      if (open) {
        //should not happen
      } else {
        if (c[-1] != '\\') {
          open = true;
          if (c+1 == segment.cend()) {
            //should not happen
          } else {
            tend = &(c[-1]);
            for (char const* cc = tbegin; cc <= tend; ++cc) {
              //TODO: apply this at the end and only do it to them if any onehas it
              if (isRegexMetacharacter(*cc)) {
                ss += '\\';
              }
              ss += *cc;
            }
            tend = nullptr;

            vbegin = &(*c);
          }
        }
      }
    } else if (*c == '>') {
      if (!open && c[-1] != '\\') {
        //should not happen
      } else {
        vend = &(*c);
        open = false;

        if (vbegin+1 == vend) {
          //should not happen
        }

        for (char const* cc = vbegin+1; cc <= vend-1; cc++) {
          if (*cc == ':') {
            vcolon = cc;
            break;
          }
        }

        ss += "(.*)";

        complexregrexFormatVariables.emplace_back(
          std::string(vbegin+1, (vcolon-1)-(vbegin+1)+1),
          std::string(vcolon+1, (vend-1)-(vcolon+1)+1)
        );

        vbegin = nullptr;
        vcolon = nullptr;
        vend = nullptr;
        tbegin = &(c[1]);
        tend = nullptr;
      }
    }
  }
  if (vbegin != nullptr || vend != nullptr) {
    fprintf(stderr, "%s\n", "Something strange has happened.");
  }

  for (char const* cc = tbegin; cc <= &(segment.end()[-1]); ++cc) {
    if (isRegexMetacharacter(*cc)) {
      ss += '\\';
    }
    ss += *cc;
  }

  //convert
  complexFormatRegex.assign(complexFormatRegexStr,
    std::regex::ECMAScript|std::regex::optimize);
}


bool RouteNode::matchesTEXT(std::string const& path_segment) const {
  return path_segment == segment_str;
}

bool RouteNode::matchesSIMPLE(std::string const& path_segment) const {
  try {
    return type_matchers.at(simpleVariable.first)(path_segment);
  } catch (std::out_of_range oor) {
    fprintf(stderr,
            "Could not find TypeMatchFunc for %s.\n",
            simpleVariable.first.c_str());
    return false;
  }
}

bool RouteNode::matchesCOMPLEX(std::string const& path_segment,
    std::unordered_map<std::string,std::string>& tmp_route_params) const {
  //uses less allocations/overall memory than smatch
  std::cmatch match;

  //each call uses ~100 allocations and ~7KB memory on normal ish complex routes
  if (std::regex_match(path_segment.c_str(), match, complexFormatRegex)) {
    for (size_t i(1); i < match.size(); i++) {
      std::string match_str;
      match_str.assign(path_segment.data() + match.position(i), match[i].length());
      try {
        if (!type_matchers.at(complexregrexFormatVariables[i-1].first)(match_str)) {
          return false;
        }
      } catch (std::out_of_range oor) {
        fprintf(stderr,
                "Could not find TypeMatchFunc for %s.\n",
                complexregrexFormatVariables[i-1].first.c_str());
        return false;
      }
      //emplace + std::move no different
      tmp_route_params[complexregrexFormatVariables[i-1].second] =
          match_str;
    }
    return true;
  }
  return false;
}



void dump(RouteNode const& r) {
  if (r.type == RouteNode::SIMPLE) {
    puts(r.simpleVariable.first.c_str());
    puts(r.simpleVariable.second.c_str());
  } else if (r.type == RouteNode::COMPLEX) {
    puts(r.complexFormatRegexStr.c_str());
    for (auto const& fv : r.complexregrexFormatVariables) {
      printf("format: %s\n", fv.first.c_str());
      printf("name: %s\n", fv.second.c_str());
    }
  }
}

int main(int argc, char** argv) {
  func_base* foo = (func_base*)1;

  RouteNode router;
  router.addRoute(argv[1], foo);

  #ifdef PPROFFING
  HeapProfilerStart("prefixx5");
  #endif
  for (int i(2); i<argc; i++) {
    std::unordered_map<std::string,std::string> route_params;
    if (router.match(argv[i], route_params)) {
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
  return 0;
}
