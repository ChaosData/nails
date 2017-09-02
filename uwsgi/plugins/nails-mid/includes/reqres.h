#ifndef _REQRES_H_
#define _REQRES_H_

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <algorithm>

#include "reqres-common.h"
#include "http.h"

#include "json.hpp"

constexpr uint64_t MAX_CONTENT_LENGTH = 2*1024*1024;

using json = nlohmann::json;

struct wsgi_request;


namespace nails {

class func_base;
class factory_base;
class RouteNode;

class Request {
 public:
  explicit Request(void) = delete;

  explicit Request(std::string&& _method,
    std::unordered_map<std::string,std::string>&& _headers,
    ParamMap&& _args,
    ParamMap&& _form,
    char const* _stream,
    FileMap&& files,
    std::unordered_map<std::string, std::string>&& _environ,
    std::string&& _origin,
    std::string&& _request_uri, //sans script_root
    std::string&& _script_root,
    std::string&& _path);

  ParamMap const form;
  ParamMap const args;
  dual_map<std::string,std::string> const values; //combined form+args
  ParamMap const& rules; //route rule variables

  std::unordered_map<std::string, std::string> const cookies;
  std::unordered_map<std::string, std::string> const headers;

  char const* stream;

  std::string const data;

  FileMap files;
  std::unordered_map<std::string, std::string> const environ;

  std::string const method;

  
  // if listening on:
  // - http://www.example.com/myapplication

  // with requested URL:
  // - http://www.example.com/myapplication/page.html?x=y
  
  std::string const origin;
  std::string const path; // :: /page.html
  std::string const script_root; // :: /myapplication

  std::string const base_url; // :: http://www.example.com/myapplication/page.html
                              // = origin + script_root + path
  
  std::string const url; // :: http://www.example.com/myapplication/page.html?x=y
                         // = origin + script_root + request_uri

  std::string const url_root; // :: http://www.example.com/myapplication/
                              // = origin + script_root

  bool const is_xhr;

  std::pair<factory_base*,func_base*> const& endpoint;

  std::pair<json const&, bool> get_json(bool force=false);

  uint64_t max_content_length;

  ~Request();

 private:
  friend bool route(struct wsgi_request* wsgi_req, std::string const& _path,
                    std::string const& _method, Request& _req);

  std::pair<factory_base*,func_base*> endpoint_;

  json json_;
  bool json_set_ = false;
  std::unordered_map<std::string, std::string> rules_;

  void set_route_rule_vars(std::unordered_map<std::string, std::string>&& _rules) {
    rules_ = std::move(_rules);
  }

  void set_endpoint(factory_base* _factory, func_base* _action) {
    endpoint_ = std::make_pair(_factory, _action);
  }

};


class Response {
  struct Cookie;

 public:
  explicit Response(void) = default;

  explicit Response(std::unordered_map<std::string, std::string>&& _headers,
    std::string const& _status, uint16_t _status_code,
    std::string&& _data);
  explicit Response(std::unordered_map<std::string, std::string>&& _headers,
    std::string&& _status,
    std::string&& _data);
  Response(std::string _data);
  Response(char const* _data);

  std::unordered_map<std::string, std::string> headers;
  std::string status;
  uint16_t status_code;

  void set_data(std::string const& _data);
  void set_data(std::string&& _data);

  std::string const& get_data() const;
  std::string mimetype = "text/html";

  void set_cookie(std::string const& key, std::string const& value="",
                  uint64_t max_age=0, uint64_t expires=0,
                  std::string const& path="/", std::string const& domain="",
                  bool secure=true, bool httponly=true);

 private:
  friend bool route(struct wsgi_request* wsgi_req, std::string const& _path,
                    std::string const& _method, Request& _req);
  friend void handleResponse(struct wsgi_request* wsgi_req, Response& res);

  std::string data_;
  std::vector<Cookie> cookies; 
};


Response make_response(std::string&& rv);
Response make_response(std::string&& rv, uint16_t status_code,
                       std::unordered_map<std::string,std::string>&& _headers);
Response make_response(std::string&& rv,
                       std::pair<uint16_t,std::string>&& _status,
                       std::unordered_map<std::string,std::string>&& _headers);

struct Response::Cookie {
  Cookie(std::string const& _key, std::string const& _value,
      uint64_t _max_age, uint64_t _expires,
      std::string const& _path, std::string const& _domain,
      bool _secure, bool _httponly);

  std::string key = "";
  std::string value = "";
  uint64_t max_age = 0;
  uint64_t expires = 0;
  std::string path = "/";
  std::string domain = "";
  bool secure = true;
  bool httponly = true;
};

};

#endif
