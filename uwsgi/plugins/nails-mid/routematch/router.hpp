#include <string>
#include <sstream>
#include <regex>
#include <unordered_map>
#include <utility>
#include <memory>

class RouteNode {
//mvp naive implementation

 public:
  RouteNode() : parent(*this), segment_str("") { }

  //goal is to recursively add routes
  void addRoute(std::string const& path);
  void addRoute(std::vector<std::string> const& segments,
                std::vector<std::string>::const_iterator cseg_iter);

  std::unordered_map<std::string,std::string> match(std::string const& urlpath);

// private:
  using FormatVariable = std::pair<std::string,std::string>;

  enum Type {
    UNINIT,
    TEXT,
    SIMPLE,
    COMPLEX
  };

  static bool containsParameters(std::string const& segment);
  static bool isValidVariable(std::string const& variable);
  static bool isValid(std::string const& segment);
  static bool isSimpleVariable(std::string const& variable);

 public:
  void extractSimpleVariable(std::string const& segment);
  void extractComplexFormat(std::string const& segment);

  Type type = UNINIT;
  std::reference_wrapper<RouteNode> parent;

  std::string segment_str;

  FormatVariable simpleVariable;
  std::vector<
    std::pair<
      std::unique_ptr<std::string>,
      std::unique_ptr<FormatVariable>
    >
  > complexFormat;

  std::unordered_map<std::string, RouteNode> children;
};

//RouteNode router;

void RouteNode::addRoute(std::string const& path) {
  std::vector<std::string> segments;
  std::stringstream ss(path);
  std::string segment;
  while (std::getline(ss, segment, '/')) {
    segments.push_back(segment);
  }

  addRoute(segments, segments.cbegin());
}


void RouteNode::addRoute(std::vector<std::string> const& segments,
                         std::vector<std::string>::const_iterator cseg_iter) {
  if (cseg_iter == segments.cend()) {
    return;
  }
  std::string const& cseg = *cseg_iter;

  if (!isValid(cseg)) {
    //TODO: something
    //note: need to validate all at once
    return;
  }

  if (!containsParameters(cseg)) {
    auto const& child_iter_search = children.find(cseg);
    std::reference_wrapper<RouteNode> child = children.begin()->second;

    //wrong. need to set self (recursively) not children
    if (child_iter_search == children.end()) {
      child = children[cseg];
      child.get().parent = *this;
      child.get().segment_str = cseg;
      child.get().type = TEXT;
    } else {
      child = children[cseg];
    }
    child.get().addRoute(segments, ++cseg_iter);
  } else {
    if (isSimpleVariable(cseg)) {
    // SIMPLE
      extractSimpleVariable(cseg);
      type = SIMPLE;
    } else {
      // COMPLEX
      // /_id-<int:id>-version-<int:version>_/
      extractComplexFormat(cseg);
      type = COMPLEX;
    }
  }
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

bool RouteNode::isValidVariable(std::string const& variable) {
  std::regex variable_regex("[a-zA-Z0-9_.]+");
  std::regex typed_variable_regex("[a-zA-Z0-9_.]+:[a-zA-Z0-9_.]+");

  return std::regex_match(variable, variable_regex)
         || std::regex_match(variable, typed_variable_regex);
}

bool RouteNode::isSimpleVariable(std::string const& variable) {
  std::regex variable_regex("<[a-zA-Z0-9_.]+>");
  std::regex typed_variable_regex("<[a-zA-Z0-9_.]+:[a-zA-Z0-9_.]+>");

  return std::regex_match(variable, variable_regex)
         || std::regex_match(variable, typed_variable_regex);
}

void RouteNode::extractSimpleVariable(std::string const& segment) {
  std::smatch match;
  std::regex variable_regex("<[a-zA-Z0-9_.]+>");
  std::regex typed_variable_regex("<[a-zA-Z0-9_.]+:[a-zA-Z0-9_.]+>");

  if (std::regex_match(segment, match, variable_regex)) {
    std::ssub_match name = match[1];
    simpleVariable = std::make_pair("", name.str());
  } else if (std::regex_match(segment, match, typed_variable_regex)) {
    std::ssub_match format = match[1];
    std::ssub_match name = match[2];
    simpleVariable = std::make_pair(format.str(), name.str());
  } else {
    simpleVariable = std::make_pair("<error>","<error>"); //illegal values
  }
}


void RouteNode::extractComplexFormat(std::string const& segment) {
  bool open = false;
  char const *tbegin, *tend, *vbegin, *vend = nullptr;

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
          }
          tend = &(c[-1]);
          vbegin = &(*c);
        }
      }
    } else if (*c == '>') {
      if (!open && c[-1] != '\\') {
        //should not happen
      } else {
        complexFormat.emplace_back(
          std::make_pair(
            std::string(tbegin, tend-tbegin+1),
            nullptr
          )
        );

        vend = &(*c);

        if (vbegin+1 == vend) {
          //should not happen
        }
        complexFormat.emplace_back(
          std::make_pair(
            nullptr,
            std::string(vbegin+1, (vend-1)-(vbegin+1)+1)
          )
        );

        vbegin = nullptr;
        vend = nullptr;
        tbegin = &(c[1]);
        tend = nullptr;
      }
    }
  }
  if (vbegin != nullptr || vend != nullptr) {
    //should not happen
  }

  //TODO: handle end of segment as text
  complexFormat.emplace_back(
    std::make_pair(
      std::string(tbegin, &(segment.end()[-1])+1),
      nullptr
    )
  );

}


void dump(RouteNode const& r) {
  if (r.type == RouteNode::SIMPLE) {
    puts(r.simpleVariable.first.c_str());
    puts(r.simpleVariable.second.c_str());
  } else if (r.type == RouteNode::COMPLEX) {

  }
}

int main() {
  RouteNode r1;
  r1.extractSimpleVariable(std::string("<foo>"));

  RouteNode r2;
  r2.extractSimpleVariable(std::string("<string:foo>"));

  RouteNode r3;
  r3.extractComplexFormat(std::string("_id-<int:id>-version-<int:version>_"));

  dump(r1);
  puts("====");
  dump(r2);
  puts("====");
  dump(r3);

  return 0;
}
