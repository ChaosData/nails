#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <algorithm>

#include "Request.h"

//only for testing
Request::Request(void) : params(query_parameters, query_parameters) { }

Request::Request(Request::Method _method,
    std::unordered_map<std::string,std::string>&& _headers,
    std::unordered_map<std::string, std::string>&& _query_parameters,
    std::unordered_map<std::string, std::string>&& _request_parameters,
    std::string const* _ip, std::string const* _remote_ip,
    std::string const* _original_method_str, std::string const* _request_method_str,
    std::string const* _base_url,
    std::string const* _original_fullpath, std::string const* _fullpath,
    uint64_t _content_length,
    char const* _raw_post,
    std::string const* _server_software
  ) : original_method_(_method), headers(std::move(_headers)),
      query_parameters(std::move(_query_parameters)),
      request_parameters(std::move(_request_parameters)),
      //yes, this is in the member list after *_parameters_
      params(query_parameters, query_parameters),
      ip_(_ip), remote_ip_(_remote_ip),
      original_method_str_(_original_method_str),
      request_method_str_(_request_method_str),
      base_url_(_base_url),
      original_fullpath_(_original_fullpath),
      fullpath_(_fullpath),
      content_length_(_content_length),
      raw_post_(_raw_post),
      server_software_(_server_software)
  { }

std::unordered_map<std::string,Request::Method> method_map = {
  {"OPTIONS", Request::Method::OPTIONS},
  {"GET", Request::Method::GET},
  {"HEAD", Request::Method::HEAD},
  {"POST", Request::Method::POST},
  {"PUT", Request::Method::PUT},
  {"DELETE", Request::Method::DELETE},
  {"TRACE", Request::Method::TRACE},
  {"CONNECT", Request::Method::CONNECT},
  {"PATCH", Request::Method::PATCH}
};
Request::Method Request::MethodForStr(std::string const& _s) {
  std::string s{_s};
  std::transform(s.begin(), s.end(), s.begin(), (int (*)(int))std::toupper);
  try {
    return method_map.at(s);
  } catch (std::out_of_range oor) { }
  return Request::Method::UNKNOWN;
}


bool Request::isOptions() { return original_method_ == Method::OPTIONS; }
bool Request::isGet() { return original_method_ == Method::GET; }
bool Request::isHead() { return original_method_ == Method::HEAD; }
bool Request::isPost() { return original_method_ == Method::POST; }
bool Request::isPut() { return original_method_ == Method::PUT; }
bool Request::isDelete() { return original_method_ == Method::DELETE; }
bool Request::isTrace() { return original_method_ == Method::TRACE; }
bool Request::isConnect() { return original_method_ == Method::CONNECT; }
bool Request::isPatch() { return original_method_ == Method::PATCH; }
bool Request::isUnknown() { return original_method_ == Method::UNKNOWN; }

bool Request::hasFormData() { return request_parameters.size() > 0; }

bool Request::isLocal() {
  //throw std::runtime_error("Not implemented");
  if (*remote_ip_ == "127.0.0.1") {
    return true;
  }
  return false;
}

bool Request::isXhr() {
  try {
    //This does not need to handle unicode
    std::string val = headers["x-requested-with"];
    std::transform(val.begin(), val.end(), val.begin(),
                   (int (*)(int))std::tolower);
    return val == "xmlhttprequest";
  } catch (std::out_of_range oor) { return false; }
}

void Request::check_path_parameters() {
  throw std::runtime_error("Not implemented");
}

std::string const& Request::authorization() {
  //if (authorization_ != nullptr) {
  if (&(authorization_.get()) != &empty) {
    //return *authorization_;
    return authorization_;
  }

  try {
    //authorization_ = &(headers.at("authorization"));
    authorization_ = headers.at("authorization");
  } catch (std::out_of_range oor) {
    //maybe replace with bitfield flags
    //authorization_ = new std::string("");
    authorization_ = *(new std::string(""));
  }
  //return *authorization_;
  return authorization_;
}

std::string const& Request::body() {
  if (body_ != nullptr) {
    return *body_;
  }
  body_ = new std::string(raw_post_, content_length_);
  return *body_;
}

uint64_t Request::content_length() { return content_length_; }

std::string const& Request::base_url() { return *base_url_; }
std::string const& Request::fullpath() { return *fullpath_; }

std::string const& Request::ip() { return *ip_; }

std::string const& Request::media_type() {
  if (media_type_ != nullptr) {
    return *media_type_;
  }
  try {
    media_type_ = &(headers.at("content-type"));
  } catch (std::out_of_range oor) {
    //maybe use a bitfield?
    media_type_ = new std::string("");
  }
  return *media_type_;
}

std::string const& Request::method() {
  if (original_method_ == Method::UNKNOWN
      || original_method_str_ != nullptr) {
    return *original_method_str_;
  }
  switch (original_method_) {
    case Method::OPTIONS:
      original_method_str_ = new std::string("OPTIONS");
      break;
    case Method::GET:
      original_method_str_ = new std::string("GET");
      break;
    case Method::HEAD:
      original_method_str_ = new std::string("HEAD");
      break;
    case Method::POST:
      original_method_str_ = new std::string("POST");
      break;
    case Method::PUT:
      original_method_str_ = new std::string("PUT");
      break;
    case Method::DELETE:
      original_method_str_ = new std::string("DELETE");
      break;
    case Method::TRACE:
      original_method_str_ = new std::string("TRACE");
      break;
    case Method::CONNECT:
      original_method_str_ = new std::string("CONNECT");
      break;
    case Method::PATCH:
      original_method_str_ = new std::string("PATCH");
      break;
    default:
      //need a good alternative for if something really strange happens so
      //that this doesn't have a memory leak. much SPACE. very JUMP. wow.
      original_method_str_ = new std::string("SPACEJUMP");
  }
  return *original_method_str_;
}

std::string const& Request::original_fullpath() { return *original_fullpath_;}

std::string const& Request::original_url() {
  if (original_url_ != nullptr) {
    return *original_url_;
  }
  std::string *ou = new std::string("");
  ou->reserve(base_url_->size() + original_fullpath_->size());
  ou->append(*base_url_);
  ou->append(*original_fullpath_);
  original_url_ = ou;
  return *original_url_;
};

char const* Request::raw_post() { return raw_post_; }

std::string const& Request::remote_ip() { return *remote_ip_; }

std::string const& Request::request_method() {
  if (request_method_str_ != nullptr) {
    return *request_method_str_;
  }
  std::string const& ret = method();
  request_method_str_ = original_method_str_;
  return ret;
}

std::string const& Request::server_software() { return *server_software_; }

Request::~Request() {
  //todo: now that this is going to take ownership of stuff passed into it,
  //we need to free all of it
  if (original_method_str_ != nullptr) delete original_method_str_;
  if (request_method_str_ != nullptr && request_method_str_ != original_method_str_) delete request_method_str_;
  if (base_url_ != nullptr) delete base_url_;
  if (original_fullpath_ != nullptr) delete original_fullpath_;
  if (fullpath_ != nullptr && fullpath_ != original_fullpath_) delete fullpath_;
  if (original_url_ != nullptr) delete original_url_;

  if (body_ != nullptr) delete body_;
  //if (authorization_ != nullptr) {
  if (&(authorization_.get()) != &empty) {
    try {
      headers.at("authorization");
    } catch (std::out_of_range oor) {
      //maybe replace with bitfield flags
      //delete authorization_;
      delete &(authorization_.get());
    }
  }
  if (ip_ != nullptr) delete ip_;
  if (remote_ip_ != nullptr) delete remote_ip_;
  if (media_type_ != nullptr) {
    try {
      headers.at("content-type");
    } catch (std::out_of_range oor) {
      //maybe replace with bitfield flags
      delete media_type_;
    }
  }
  //if (raw_post_ != nullptr) std::free((void*)raw_post_);
  if (server_software_ != nullptr) delete server_software_;
}
