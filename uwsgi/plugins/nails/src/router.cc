#include "router.h"
#include "reqres.h"
#include "uwsgi-includer.h"
#include "filters.h"
#include "config.h"


#include <stdexcept>
#include <string>
#include <memory>
#include <time.h>
#include <iomanip>


namespace nails {

//RouteNode router{};
std::unordered_map<std::string,RouteNode> router{};

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




bool RouteNode::add(std::string&& _path, std::set<std::string>&& _methods,
                           func_base* _action, char const* _controller) {
  std::vector<std::string> segments = split_escape(_path, '/');

  factory_base* factory = controller_factory_mapper.at(_controller);

  //validate entire path
  for (auto const& seg : segments) {
    if (!isValid(seg)) {
      return false;
    }
  }

  for (auto const& m : _methods) {
    router[m].add(segments, segments.cbegin(), std::set<std::string>(_methods),
                  _action, factory);

  }

  //router.add(segments, segments.cbegin(), std::move(_methods), _action, factory);
  return true;
}


void RouteNode::add(std::vector<std::string> const& segments,
                    std::vector<std::string>::const_iterator cseg_iter,
                    std::set<std::string>&& methods,
                    func_base* action, factory_base* factory) {
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
    child.get().add(segments, cseg_iter+1, std::move(methods), action, factory);
  } else {
    if (action_ != nullptr) {
      std::string fullpath;
      for (auto const& seg : segments) {
        fullpath += "/" + seg;
      }
      fprintf(stderr, "Duplicate route occurred. Overwriting route for %s.\n",
              fullpath.c_str());
    }
    action_ = action;
    factory_ = factory;
    methods_ = std::move(methods);
  }
}



std::pair<factory_base*,func_base*>
RouteNode::match(std::string const& path,
                 std::string const& _method,
                 std::unordered_map<std::string,
                                    std::string>& route_params) {

  printf("path: %s\n", path.c_str());
  printf("method: %s\n", _method.c_str());
  std::vector<std::string> segments = split_decode(path, '/');

  return router[_method].match(segments, segments.cbegin(), _method, route_params);
}

std::pair<factory_base*,func_base*>
RouteNode::match(std::vector<std::string> const& segments,
                 std::vector<std::string>::const_iterator cseg_iter,
                 std::string const& _method,
                 std::unordered_map<std::string,
                                    std::string>& route_params) const {

  std::string const& cseg = *cseg_iter;

  printf("path segment: %s\n", cseg_iter->c_str());
  printf("route segment: %s\n", segment_str.c_str());
  switch (type) {
    case TEXT: {
      if (!matchesTEXT(cseg)) {
        return std::make_pair<factory_base*,func_base*>(nullptr,nullptr);
      }
      break;
    }
    case SIMPLE: {
      if (!matchesSIMPLE(cseg)) {
        return std::make_pair<factory_base*,func_base*>(nullptr,nullptr);
      }
      break;
    }
    case COMPLEX: {
      if (!matchesCOMPLEX(cseg, route_params)) {
        return std::make_pair<factory_base*,func_base*>(nullptr,nullptr);
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
      return std::make_pair<factory_base*,func_base*>(nullptr,nullptr);
    }
  }

  //match children if further path segments exist
  //store return value temporarily (if not nullptr) to extract params
  std::pair<factory_base*,func_base*> ret{nullptr,nullptr};
  if (cseg_iter+1 != segments.end()) {
    bool found = false;
    for (auto const& kv : children) {
      ret = kv.second.match(segments, cseg_iter+1, _method, route_params);
      if (ret.second != nullptr) {
        found = true;
        break;
      }
    }
  } else {
    //something weird happens here with make_pair and it errors out as if the
    //args were const*
    ret = std::make_pair<factory_base*,func_base*>(
            const_cast<factory_base*>(factory_),
            const_cast<func_base*>(action_)
          );
    //will be nullptr,nullptr if this is not a matchable node
  }

  //extract (or remove) route params
  switch (type) {
    case UNINIT: {
      //this is here to shut the compiler up
      return std::make_pair<factory_base*,func_base*>(nullptr,nullptr);
    }
   case TEXT: {
      break;
    }
    case SIMPLE: {
      if (ret.second != nullptr) {
        route_params[simpleVariable.second] = cseg;
      }
      break;
    }
    case COMPLEX: {
      if (ret.second == nullptr) {
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
    if (*c == '<' && c[-1] != '\\') {
      if (open) {
        return false;
      }
      open = true;
      if (c+1 == segment.cend()) {
        return false;
      }
      vbegin = &(*c);
    } else if (*c == '>' && c[-1] != '\\') {
      if (!open) {
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


inline bool isRegexMetacharacter(char c) {
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
    if (*c == '<' && c[-1] != '\\') {
      if (open) {
        //should not happen
      } else {
        open = true;
        if (c+1 == segment.cend()) {
          //should not happen
        }
        tend = &(c[-1]);

        //the set of non-variable chars before each variable
        for (char const* cc = tbegin; cc <= tend; ++cc) {
          //TODO: apply this at the end and only do it to them if any onehas it
          if (isRegexMetacharacter(*cc)) { // "aaaaaa\\<"
            if (*cc != '\\') {
              ss += '\\';
            } else if (cc[1] != '<' && cc[1] != '>') {
              ss += '\\';
            }
          }
          ss += *cc;
        }
        tend = nullptr;

        vbegin = &(*c);
      }
    } else if (*c == '>' && c[-1] != '\\') {
      if (!open) {
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
      if (*cc != '\\') {
        ss += '\\';
      }
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


bool testHookPre(Request& req) {
  if (req.args.find("testHookPre") != req.args.end()) {
    return true;
  }
  return false;
}

Response doTestHookPre(Request&) {
  return "testHookPre";
}

bool testHookPost(Request& req) {
  if (req.args.find("testHookPost") != req.args.end()) {
    return true;
  }
  return false;
}

Response doTestHookPost(Request& req) {
  return std::string("doTestHookPost: ") + req.url;
}


void handleResponse(struct wsgi_request* wsgi_req, Response& res) {
  uwsgi_response_prepare_headers(wsgi_req,
                                 (char*)res.status.data(),
                                 res.status.size());
  for (auto const& kv : res.headers) {
    if (kv.first == "x-sendfile" && !upstream_x_sendfile) {}
    uwsgi_response_add_header(wsgi_req,
                              (char*)kv.first.data(), kv.first.size(),
                              (char*)kv.second.data(), kv.second.size());
  }


  for (auto const& c : res.cookies) {
    std::string cookie = encodeURIComponent(c.key) + "="
                         + encodeURIComponent(c.value)
                        + ";" + " Path=" + c.path;
    if (c.expires > 0 && c.max_age == std::numeric_limits<uint64_t>::max()) {
      std::stringstream expires;
      struct tm t{};
      struct tm* tp = gmtime_r(const_cast<std::time_t*>(
                                reinterpret_cast<std::time_t const*>(
                                  &(c.expires)
                                )
                              ), &t
      );
      expires.imbue(std::locale("en_US.utf8"));
      expires << std::put_time(tp, "%a, %d %b %Y %H:%M:%S GMT");
      cookie += "; Expires=" + expires.str();
    } else if (c.max_age != std::numeric_limits<uint64_t>::max()) {
      cookie += "; Max-Age=" + std::to_string(c.max_age);
    }

    if (!c.domain.empty()) {
      cookie += "; Domain=" + c.domain;
    }

    if (c.secure) {
      cookie += "; Secure";
    }

    if (c.httponly) {
      cookie += "; HttpOnly";
    }

    uwsgi_response_add_header(wsgi_req,
                              (char*)"Set-Cookie", sizeof("Set-Cookie")-1,
                              (char*)cookie.data(), cookie.size());
  }


  auto xsendfile = res.headers.find("x-sendfile");
  if (xsendfile != res.headers.end()
      && res.data_.size() == 0 && !upstream_x_sendfile) {
    // data.size() check is to prevent header injection->file extraction
    std::string const& fp = xsendfile->second;
    int fd;
    struct stat stat_buf;
    fd = open(fp.c_str(), O_RDONLY);
    fstat(fd, &stat_buf);
    uwsgi_response_sendfile_do(wsgi_req, fd, 0, stat_buf.st_size);
  } else {
    uwsgi_response_write_body_do(wsgi_req, (char*)res.data_.data(),
                                 res.data_.size());
  }
}


    // prematch_filters.emplace_back(std::make_pair(
    //                                 make_func(&testHookPre),
    //                                 make_func(&doTestHookPre))
    // );

    // postmatch_filters.emplace_back(std::make_pair(
    //                                 make_func(&testHookPost),
    //                                 make_func(&doTestHookPost))
    // );


bool route(struct wsgi_request* wsgi_req, std::string const& _path,
                      std::string const& _method, Request& _req) {

  bool skip = false;
  bool skip404 = false;
  BaseController* bcp = nullptr;
  Response res;

  //run prematch filters
  for (auto const& p : prematch_filters) {
    bool end = (*(p.first))(_req);
    if (end) {
      res = (*(p.second))(_req);
      skip = true;
      break;
    }
  }

  factory_base* factory;
  func_base* action;
  if (!skip) {
    std::unordered_map<std::string,std::string> route_params;

    std::tie(factory, action) = RouteNode::match(_path, _method, route_params);

    if (action == nullptr) {
      std::string p = _req.path.substr(1, _req.path.size());

      char const* cname = nullptr;
      char const* mname = nullptr;

      std::vector<std::string> parts = split(p, '/');
      cname = parts[0].c_str();
      if (parts.size() == 1) {
        mname = "index";
      } else if (parts.size() == 2) {
        mname = parts[1].c_str();
      } else {
        printf("[??] Can't handle %s %s\n", _req.method.c_str(), _req.url.c_str());
      }

      ControllerActionMapper* cam = nullptr;
      try {
        cam = &(controller_mapper.at(cname));
      } catch (std::out_of_range oor) {
        try {
          cname = controller_name_mapper.at(std::string(cname));
          cam = &(controller_mapper.at(cname));
        } catch (std::out_of_range oor) {
          puts("Couldn't find the controller!");
        }
      }
      if (cam != nullptr) {
        action = cam->get(mname);
        try {
          factory = controller_factory_mapper.at(cname);
        } catch (std::out_of_range oor) {}
      }
    }

    _req.set_endpoint(factory, action);
    _req.set_route_rule_vars(std::move(route_params));

    //run postmatch filters
    for (auto const& p : postmatch_filters) {
      bool end = (*(p.first))(_req);
      if (end) {
        res = (*(p.second))(_req);
        skip = true;
        skip404 = true;
        break;
      }
    }

    std::tie(factory, action) = _req.endpoint; 
    if (action == nullptr || factory == nullptr) {
      skip = true;
    }

    if (!skip) {
      bcp = factory->create();
      if (bcp != nullptr) {
        auto* mf = static_cast<func<Response(BaseController*,Request&)>*>(
            action
        );

        try {
          res = (*mf)(std::move(bcp), _req);
        } catch (...) {
          res.status = "500";
          res.data_ = "";
        }
        delete bcp;
      }
    }

    if (!skip404) {
      if (action == nullptr) {
        res = make_response("", 404, {});
      }
    }
  }

  //run response filters
  for (auto const& f : response_filters) {
    bool end = (*f)(_req, res);
    if (end) {
      handleResponse(wsgi_req, res);
      return true;
    }
  }
  handleResponse(wsgi_req, res);
  return true;
}

};