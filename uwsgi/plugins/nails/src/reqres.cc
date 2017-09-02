#include "reqres.h"
#include "signverify.h"
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
                 ParamMap&& _headers,
                 ParamMap&& _cookies,
                 ParamMap&& _args,
                 ParamMap&& _form,
                 char const* _stream,
                 std::unordered_map<std::string, FileStorage>&& _files,
                 ParamMap&& _environ,
                 std::string&& _origin,
                 std::string&& _request_uri, //sans script_root
                 std::string&& _script_root,
                 std::string&& _path) :
  form_(std::move(_form)),
  args_(std::move(_args)),
  values_(form_, args_),
  //rules_(_rules),
  headers_(std::move(_headers)),
  cookies_(std::move(_cookies)),
  stream_(_stream == nullptr ? nullptr :
                              setStreamData(headers) ? _stream :
                                                       nullptr),
  data_(stream == nullptr ? "" :
    std::string(
      _stream,
      static_cast<size_t>(atoll(headers.at("content-length").c_str()))
    )
  ),
  files_(std::move(_files)),
  environ_(std::move(_environ)),
  method_(std::move(_method)),
  origin_(std::move(_origin)),
  path_(std::move(_path)),
  script_root_(std::move(_script_root)),
  base_url_(origin + script_root + path),
  url_(origin + script_root + _request_uri),
  url_root_(origin + script_root),
  is_xhr_(isXHR(headers)),
  endpoint_(endpoint_)
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

std::string const& Request::get_csrf_token() {
  if (csrf_token_ != nullptr) {
    return *csrf_token_;
  } else if (mapHas(session, "csrf_token")) {
    csrf_token_ = session["csrf_token"].get_ptr<std::string const*>();
    return *csrf_token_;
  } else {
    if (reset_csrf_token()) {
      return *csrf_token_;
    } else {
      throw std::runtime_error("Failed to generate CSRF token.");
    }
  }
  return *csrf_token_;
}

bool Request::reset_csrf_token() {
  uint8_t key[crypto_auth_KEYBYTES];
  std::string key_hex;
  key_hex.resize(nails::key_hex_len - 1);
  if (nails::generate_key_and_hex(key, &key_hex[0])) {
    session["csrf_token"] = std::move(key_hex);
    csrf_token_ = session["csrf_token"].get_ptr<std::string const*>();
    return true;
  }
  return false;
}


Request::~Request() { }




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
  status += " " + _status;
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

Response::Response(json const& _data) :
    Response(
      std::unordered_map<std::string, std::string>{
        {"Content-Type", "application/json; charset=UTF-8"}
      },
      "200 OK", _data.dump()
    )
{}


/*
Response make_response(std::string&& rv) {
  return Response(
    std::unordered_map<std::string, std::string>{
      {"Content-Type", "text/html; charset=UTF-8"}
    },
    "200 OK", std::move(rv));
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


Response make_response(json&& rv) {
  return Response(
    std::unordered_map<std::string, std::string>{
      {"Content-Type", "text/html; charset=UTF-8"}
    },
    "200 OK", rv.dump());
}

Response make_response(json&& rv, uint16_t status_code,
    std::unordered_map<std::string,std::string>&& _headers) {
  return Response(
    std::move(_headers),
    getTextForStatus(status_code), status_code, rv.dump());
}

Response make_response(json&& rv,
    std::pair<uint16_t,std::string>&& _status,
    std::unordered_map<std::string,std::string>&& _headers) {
  return Response(
    std::move(_headers),
    std::move(_status.second), _status.first, rv.dump());
}
*/

Response send_file(std::string const& filepath,
                   std::string const& content_type,
                   bool as_attachment,
                   std::string const& attachment_filename) {
  std::unordered_map<std::string,std::string> headers = {
      {"x-sendfile", filepath},
      {"Content-Type", content_type}
  };
  if (as_attachment) {
    std::string efn = encodeURIComponent(attachment_filename);
    headers.emplace(
      std::make_pair(
        "Content-Disposition",
        std::string{"attachment; filename=\""} + efn + "\"; filename*=UTF-8''"
          + efn
      )
    );
  }
  return Response(
    std::move(headers),
    "200 OK", "" //note: there are irrelevant 
  );
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
