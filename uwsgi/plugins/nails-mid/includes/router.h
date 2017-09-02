#ifndef _ROUTER_H_
#define _ROUTER_H_

#include <string>
#include <vector>
#include <regex>
#include <utility>
#include <unordered_map>
#include <memory>

#include "common.h"
#include "controller-helper.h"

namespace nails {

using TypeMatchFunc = bool (*) (std::string const&);
using FormatVariable = std::pair<std::string,std::string>;

extern std::unordered_map<std::string,TypeMatchFunc> type_matchers;

bool stringMatcher(std::string const&);

extern std::regex intMatcher_regex;
bool intMatcher(std::string const& path_segment);

extern std::regex floatMatcher_regex;
bool floatMatcher(std::string const& path_segment);

//std::vector<std::string> split(std::string const& original, char separator);




class RouteNode {
//mvp naive implementation
//hold entire route structure for all routes
 public:
  RouteNode() : parent(*this), segment_str("") { }

  // void add(std::string&& path, std::set<std::string>&& methods,
  //          func_base* action, char const* controller)

  //goal is to recursively add routes
  static bool add(std::string&& _path, std::set<std::string>&& _methods,
                  func_base* _action, char const* _controller);

  static std::pair<factory_base*,func_base*>
  match(std::string const& path, std::string const& _method,
        std::unordered_map<std::string,std::string>& route_params);

  enum Type {
    UNINIT,
    TEXT,
    SIMPLE,
    COMPLEX
  };


 private:
  void add(std::vector<std::string> const& segments,
           std::vector<std::string>::const_iterator cseg_iter,
           std::set<std::string>&& methods,
           func_base* action, factory_base* controller);
  
  std::pair<factory_base*,func_base*>
  match(std::vector<std::string> const& segments,
        std::vector<std::string>::const_iterator cseg_iter,
        std::string const& _method,
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

  void extractSimpleVariableRoute(std::string const& segment);
  void extractComplexFormatRoute(std::string const& segment);

  func_base* action_ = nullptr;
  factory_base* factory_ = nullptr;
  std::set<std::string> methods_;

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

extern std::unordered_map<std::string,RouteNode> router;


bool route(struct wsgi_request* wsgi_req, std::string const& _path,
           std::string const& _method, Request& _req);

};

#endif
