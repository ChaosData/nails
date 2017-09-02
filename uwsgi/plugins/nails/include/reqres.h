#ifndef _REQRES_H_
#define _REQRES_H_

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <algorithm>
#include <limits>
#include <memory>
#include <type_traits>
#include <map>

#include "reqres-common.h"
#include "http.h"

#include "json.hpp"

constexpr uint64_t MAX_CONTENT_LENGTH = 2*1024*1024;

struct wsgi_request;


namespace nails {

using json = nlohmann::json;

class func_base;
class factory_base;
class RouteNode;

class Request {
 public:
  explicit Request(void) = delete;

  explicit Request(std::string&& _method,
    ParamMap&& _headers,
    ParamMap&& _cookies,
    ParamMap&& _args,
    ParamMap&& _form,
    char const* _stream,
    FileMap&& files,
    ParamMap&& _environ,
    std::string&& _origin,
    std::string&& _request_uri, //sans script_root
    std::string&& _script_root,
    std::string&& _path);

  ParamMap const& form = form_;
  ParamMap const& args = args_;
  dual_map<std::string,std::string> const& values = values_; //combined form+args
  ParamMap const& rules = rules_; //route rule variables

  std::unordered_map<std::string, std::string> const& headers = headers_;
  std::unordered_map<std::string, std::string> const& cookies = cookies_;
  json session{};

  char const* const& stream = stream_;

  std::string const& data = data_;

  FileMap const& files = files_;
  std::unordered_map<std::string, std::string> const& environ = environ_;

  std::string const& method = method_;

  
  // if listening on:
  // - http://www.example.com/myapplication

  // with requested URL:
  // - http://www.example.com/myapplication/page.html?x=y
  
  std::string const& origin = origin_;
  std::string const& path = path_; // :: /page.html
  std::string const& script_root = script_root_; // :: /myapplication

  std::string const& base_url = base_url_; // :: http://www.example.com/myapplication/page.html
                              // = origin + script_root + path
  
  std::string const& url = url_; // :: http://www.example.com/myapplication/page.html?x=y
                         // = origin + script_root + request_uri

  std::string const& url_root = url_root_; // :: http://www.example.com/myapplication/
                              // = origin + script_root

  bool const& is_xhr = is_xhr_;

  std::pair<factory_base*,func_base*> const& endpoint = endpoint_;

  std::pair<json const&, bool> get_json(bool force=false);

  std::string const& get_csrf_token();

  bool reset_csrf_token();


  uint64_t max_content_length;

  ~Request();

 private:
  friend bool route(struct wsgi_request* wsgi_req, std::string const& _path,
                    std::string const& _method, Request& _req);
 protected:
  ParamMap form_;
  ParamMap args_;
  dual_map<std::string,std::string> values_; //combined form+args
  ParamMap rules_; //route rule variables

  ParamMap headers_;
  ParamMap cookies_;

  char const* stream_;

  std::string data_;

  FileMap files_;
  ParamMap environ_;

  std::string method_;

  
  // if listening on:
  // - http://www.example.com/myapplication

  // with requested URL:
  // - http://www.example.com/myapplication/page.html?x=y
  
  std::string origin_;
  std::string path_; // :: /page.html
  std::string script_root_; // :: /myapplication

  std::string base_url_; // :: http://www.example.com/myapplication/page.html
                              // = origin + script_root + path
  
  std::string url_; // :: http://www.example.com/myapplication/page.html?x=y
                         // = origin + script_root + request_uri

  std::string url_root_; // :: http://www.example.com/myapplication/
                              // = origin + script_root

  bool is_xhr_;

  std::pair<factory_base*,func_base*> endpoint_;

  json json_;
  bool json_set_ = false;

  std::string const* csrf_token_ = nullptr;

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
  Response(json const& _data);

  std::unordered_map<std::string, std::string> headers;
  std::string status;
  uint16_t status_code;

  void set_data(std::string const& _data);
  void set_data(std::string&& _data);

  std::string const& get_data() const;
  std::string mimetype = "text/html";

  void set_cookie(std::string const& key, std::string const& value="",
                  uint64_t max_age=std::numeric_limits<uint64_t>::max(),
                  uint64_t expires=0,
                  std::string const& path="/", std::string const& domain="",
                  bool secure=true, bool httponly=true);

 private:
  friend bool route(struct wsgi_request* wsgi_req, std::string const& _path,
                    std::string const& _method, Request& _req);
  friend void handleResponse(struct wsgi_request* wsgi_req, Response& res);

  std::string data_;
  std::vector<Cookie> cookies; 
};

inline std::string getTextForStatus(uint16_t status_code) {
  switch (status_code) {
    case 100: return " Continue";
    case 200: return " OK";
    case 201: return " Created";
    case 204: return " No Content";
    case 301: return " Moved Permanently";
    case 302: return " Found";
    case 303: return " See Other";
    case 304: return " Not Modified";
    case 307: return " Temporary Redirect";
    case 308: return " Permanent Redirect";
    case 400: return " Bad Request";
    case 401: return " Unauthorized";
    case 403: return " Forbidden";
    case 404: return " Not Found";
    case 405: return " Method Not Allowed";
    case 406: return " Not Accessible";
    case 410: return " Gone";
    case 415: return " Unsupported Media Type";
    case 418: return " \x49\x27\x6d\x20\x61\x20\x74\x65\x61\x70\x6f\x74";
    case 422: return " Unprocessable Entity";
    case 429: return " Too Many Requests";
    case 451: return " Unavailable For Legal Reasons";
    case 500: return " Internal Server Error"; //aka smoke
    case 501: return " Not Implemented";
    case 502: return " Bad Gateway";
    case 503: return " Service Unavailable";
    case 504: return " Gateway Timeout";
    case 505: return " HTTP Version Not Supported";
    case 507: return " Insufficient Storage";
    default:
      return "WAT"; // "Siggity Swat All Of My WAT" is just too long, y'know?
  }
}


/*
template<typename T>
Response make_response(T&& rv) {
  if (std::is_same<T, std::string>::value) {
    return Response(
      std::unordered_map<std::string, std::string>{
        {"Content-Type", "text/html; charset=UTF-8"}
      },
      "200 OK", std::move(rv)
    );
  } else if (std::is_same<T, json>::value) {
    return Response(
      std::unordered_map<std::string, std::string>{
        {"Content-Type", "text/html; charset=UTF-8"}
      },
      "200 OK", rv.dump()
    );
  } else {
    static_assert(!(std::is_same<T, std::string>::value
                    || std::is_same<T, json>::value),
                  "Invalid make_response type.");
  }
}


template<typename T>
Response make_response(T&& rv, uint16_t status_code,
    std::unordered_map<std::string,std::string>&& _headers) {
  if (std::is_same<T, std::string>::value) {
    return Response(
      std::move(_headers),
      getTextForStatus(status_code), status_code, std::move(rv)
    );
  } else if (std::is_same<T, json>::value) {
    return Response(
      std::move(_headers),
      getTextForStatus(status_code), status_code, rv.dump()
    );
  } else {
    static_assert(!(std::is_same<T, std::string>::value
                    || std::is_same<T, json>::value),
                  "Invalid make_response type.");
  }
}


template<typename T>
Response make_response(T&& rv,
    std::pair<uint16_t,std::string>&& _status,
    std::unordered_map<std::string,std::string>&& _headers) {
  if (std::is_same<T, std::string>::value) {
    return Response(
      std::move(_headers),
      std::move(_status.second), _status.first, std::move(rv)
    );
  } else if (std::is_same<T, json>::value) {
    return Response(
      std::move(_headers),
      std::move(_status.second), _status.first, rv.dump()
    );
  } else {
    static_assert(!(std::is_same<T, std::string>::value
                    || std::is_same<T, json>::value),
                  "Invalid make_response type.");
  }
}
*/


template<
  typename T,
  std::enable_if_t<
    std::is_same<T, std::string>::value
    || std::is_same<char const *, typename std::decay<T>::type>::value
    || std::is_same<char *, typename std::decay<T>::type>::value
  >* = nullptr
>
Response make_response(T&& rv) {
  return Response(
    std::unordered_map<std::string, std::string>{
      {"Content-Type", "text/html; charset=UTF-8"}
    },
    "200 OK", std::move(rv)
  );
}

template<
  typename T,
  std::enable_if_t<
    !(std::is_same<T, std::string>::value
    || std::is_same<char const *, typename std::decay<T>::type>::value
    || std::is_same<char *, typename std::decay<T>::type>::value)
  >* = nullptr
>
Response make_response(T&& rv) {
  return Response(
    std::unordered_map<std::string, std::string>{
      {"Content-Type", "text/html; charset=UTF-8"}
    },
    "200 OK", rv.dump()
  );
}


template<
  typename T,
  std::enable_if_t<
    std::is_same<T, std::string>::value
    || std::is_same<char const *, typename std::decay<T>::type>::value
    || std::is_same<char *, typename std::decay<T>::type>::value
  >* = nullptr
>
Response make_response(T&& rv, uint16_t status_code,
    std::unordered_map<std::string,std::string>&& _headers) {
  return Response(
    std::move(_headers),
    getTextForStatus(status_code), status_code, std::move(rv)
  );
}


template<
  typename T,
  std::enable_if_t<
    !(std::is_same<T, std::string>::value
    || std::is_same<char const *, typename std::decay<T>::type>::value
    || std::is_same<char *, typename std::decay<T>::type>::value)
  >* = nullptr
>
Response make_response(T&& rv, uint16_t status_code,
  std::unordered_map<std::string,std::string>&& _headers) {
  return Response(
    std::move(_headers),
    getTextForStatus(status_code), status_code, rv.dump()
  );
}


template<
  typename T,
  std::enable_if_t<
    std::is_same<T, std::string>::value
    || std::is_same<char const *, typename std::decay<T>::type>::value
    || std::is_same<char *, typename std::decay<T>::type>::value
  >* = nullptr
>
Response make_response(T&& rv,
    std::pair<uint16_t,std::string>&& _status,
    std::unordered_map<std::string,std::string>&& _headers) {
  return Response(
    std::move(_headers),
    std::move(_status.second), _status.first, std::move(rv)
  );
}

template<
  typename T,
  std::enable_if_t<
    !(std::is_same<T, std::string>::value
    || std::is_same<char const *, typename std::decay<T>::type>::value
    || std::is_same<char *, typename std::decay<T>::type>::value)
  >* = nullptr
>
Response make_response(T&& rv,
    std::pair<uint16_t,std::string>&& _status,
    std::unordered_map<std::string,std::string>&& _headers) {
  return Response(
    std::move(_headers),
    std::move(_status.second), _status.first, rv.dump()
  );
}



/*
Response make_response(std::string&& rv);
Response make_response(std::string&& rv, uint16_t status_code,
                       std::unordered_map<std::string,std::string>&& _headers);
Response make_response(std::string&& rv,
                       std::pair<uint16_t,std::string>&& _status,
                       std::unordered_map<std::string,std::string>&& _headers);

Response make_response(json&& rv);
Response make_response(json&& rv, uint16_t status_code,
                       std::unordered_map<std::string,std::string>&& _headers);
Response make_response(json&& rv,
                       std::pair<uint16_t,std::string>&& _status,
                       std::unordered_map<std::string,std::string>&& _headers);
*/

Response send_file(std::string const& filepath,
                   std::string const& content_type,
                   bool as_attachment = false,
                   std::string const& attachment_filename = "download");

struct Response::Cookie {
  Cookie(std::string const& _key, std::string const& _value,
      uint64_t _max_age, uint64_t _expires,
      std::string const& _path, std::string const& _domain,
      bool _secure, bool _httponly);

  std::string key = "";
  std::string value = "";
  uint64_t max_age = std::numeric_limits<uint64_t>::max();
  uint64_t expires = 0;
  std::string path = "/";
  std::string domain = "";
  bool secure = true;
  bool httponly = true;
};

};

#endif
