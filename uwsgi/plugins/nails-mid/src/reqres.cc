#include "reqres.h"
#include <set>
#include <type_traits>


namespace nails {

inline bool isXHR(std::unordered_map<std::string,std::string> const& headers) {
  auto search = headers.find("x-requested-with");
  if (search != headers.end()) {
    if (search->second == "XMLHttpRequest") {
      return true;
    }
  }
  return false;
}

std::set<std::string> nonStreamData_content_types{
  "application/x-www-form-urlencoded",
  "multipart/form-data",
  "application/json"
};

bool setStreamData(std::unordered_map<std::string,
                                      std::string> const& headers) {
  auto content_type_header = headers.find("content-type");
  if (content_type_header == headers.end()) {
    return true;
  }
  std::string const& content_type_full = content_type_header->second;
  size_t semicolon = content_type_full.find(";");

  std::string content_type{content_type_full,
                           semicolon != std::string::npos ?
                             semicolon : content_type_full.size()};
  return nonStreamData_content_types.find(content_type)
         != nonStreamData_content_types.end();
}


std::string applicationjson{"application/json"};
bool isJSON(std::unordered_map<std::string, std::string> const& headers) {
  auto content_type_header = headers.find("content-type");
  if (content_type_header == headers.end()) {
    return false;
  }
  std::string const& content_type = content_type_header->second;
  if (startsWith(content_type, applicationjson)) {
    if (content_type.size() == applicationjson.size()) {
      return true;
    }
    if (content_type[applicationjson.size()] == ';') {
      return true;
    } // not gonna bother checking for spaces. send a proper request or don't.
  }
  return false;
}

Request::Request(std::string&& _method,
                 std::unordered_map<std::string,std::string>&& _headers,
                 std::unordered_map<std::string, std::string>&& _args,
                 std::unordered_map<std::string, std::string>&& _form,
                 char const* _stream,
                 std::unordered_map<std::string, FileStorage>&& _files,
                 std::unordered_map<std::string, std::string>&& _environ,
                 std::string&& _origin,
                 std::string&& _request_uri, //sans script_root
                 std::string&& _script_root,
                 std::string&& _path) :
  form(std::move(_form)),
  args(std::move(_args)),
  values(form, args),
  rules(rules_),
  headers(std::move(_headers)),
  stream(_stream == nullptr ? nullptr :
                              setStreamData(headers) ? _stream :
                                                       nullptr),
  data(stream == nullptr ? "" :
    std::string(
      _stream,
      static_cast<size_t>(atoll(headers.at("content-length").c_str()))
    )
  ),
  files(std::move(_files)),
  environ(std::move(_environ)),
  method(std::move(_method)),
  origin(std::move(_origin)),
  path(std::move(_path)),
  script_root(std::move(_script_root)),
  base_url(origin + script_root + path),
  url(origin + script_root + _request_uri),
  url_root(origin + script_root),
  is_xhr(isXHR(headers)),
  endpoint(endpoint_)
{ }

std::pair<json const&, bool> Request::get_json(bool force) {
  if (json_set_) {
    return std::make_pair<json const&, bool>(json_, true);
  }

  bool valid = false;
  if(isJSON(headers) || force) {
    try {
      json_ = json::parse(data); // note, this doesn't have to be an object
      valid = true;
    } catch (...) { }
  }
  return std::make_pair<json const&, bool>(json_, std::move(valid));
}


Request::~Request() { }


std::string getTextForStatus(uint16_t status_code) {
  switch (status_code) {
    case 100: return "100 Continue";
    case 200: return "200 OK";
    case 201: return "201 Created";
    case 204: return "204 No Content";
    case 301: return "301 Moved Permanently";
    case 302: return "302 Found";
    case 303: return "303 See Other";
    case 304: return "304 Not Modified";
    case 307: return "307 Temporary Redirect";
    case 308: return "308 Permanent Redirect";
    case 400: return "400 Bad Request";
    case 401: return "401 Unauthorized";
    case 403: return "403 Forbidden";
    case 404: return "404 Not Found";
    case 405: return "405 Method Not Allowed";
    case 406: return "406 Not Accessible";
    case 410: return "410 Gone";
    case 415: return "415 Unsupported Media Type";
    case 418: return "\x34\x31\x38\x20\x49\x27\x6d\x20\x61\x20\x74\x65\x61\x70\x6f\x74";
    case 422: return "422 Unprocessable Entity";
    case 429: return "429 Too Many Requests";
    case 451: return "451 Unavailable For Legal Reasons";
    case 500: return "500 Internal Server Error"; //aka smoke
    case 501: return "501 Not Implemented";
    case 502: return "502 Bad Gateway";
    case 503: return "503 Service Unavailable";
    case 504: return "504 Gateway Timeout";
    case 505: return "505 HTTP Version Not Supported";
    case 507: return "507 Insufficient Storage";
    default:
      return "999 WAT"; // "Siggity Swat All Of My WAT" is just too long, y'know?
  }
}

Response::Cookie::Cookie(std::string const& _key, std::string const& _value,
    uint64_t _max_age, uint64_t _expires,
    std::string const& _path, std::string const& _domain,
    bool _secure, bool _httponly) :
      key(_key), value(_value), max_age(_max_age), expires(_expires),
      path(_path), domain(_domain), secure(_secure), httponly(_httponly)
{ }


Response::Response(std::unordered_map<std::string, std::string>&& _headers,
                   std::string const& _status, uint16_t _status_code,
                   std::string&& _data) :
    headers(std::move(_headers)),
    status(std::to_string(_status_code)),
    data_(std::move(_data))  
{
  status += _status;
}

Response::Response(std::unordered_map<std::string, std::string>&& _headers,
                   std::string&& _status,
                   std::string&& _data) :
    headers(std::move(_headers)),
    status(std::move(_status)),
    data_(std::move(_data))  
{
}

Response::Response(std::string _data) :
    Response(
      std::unordered_map<std::string, std::string>{
        {"Content-Type", "text/html; charset=UTF-8"}
      },
      "200 OK", std::move(_data)
    )
{}

Response::Response(char const* _data) :
    Response(
      std::unordered_map<std::string, std::string>{
        {"Content-Type", "text/html; charset=UTF-8"}
      },
      "200 OK", std::string(_data)
    )
{}




Response make_response(std::string&& rv) {
  return Response(
    std::unordered_map<std::string, std::string>{
      {"Content-Type", "text/html; charset=UTF-8"}
    },
    "OK", 200, std::move(rv));
}

Response make_response(std::string&& rv, uint16_t status_code,
    std::unordered_map<std::string,std::string>&& _headers) {
  return Response(
    std::move(_headers),
    getTextForStatus(status_code), status_code, std::move(rv));
}

Response make_response(std::string&& rv,
    std::pair<uint16_t,std::string>&& _status,
    std::unordered_map<std::string,std::string>&& _headers) {
  return Response(
    std::move(_headers),
    std::move(_status.second), _status.first, std::move(rv));
}


void Response::set_data(std::string const& _data) {
  data_ = _data;
}

void Response::set_data(std::string&& _data) {
  data_ = std::move(_data);
}


std::string const& Response::get_data() const {
  return data_;
}


void Response::set_cookie(
    std::string const& key, std::string const& value,
    uint64_t max_age, uint64_t expires,
    std::string const& path, std::string const& domain,
    bool secure, bool httponly) {
  //Cookie c;
  cookies.emplace_back(key, value, max_age, expires,
                       path, domain, secure, httponly);
}


};
