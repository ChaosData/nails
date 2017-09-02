#ifndef _REQUEST_H_
#define _REQUEST_H_

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <algorithm>

#include "reqres-common.h"

class Request {
 public:
  enum class Method { OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE, CONNECT, PATCH, SPACEJUMP, UNKNOWN };

  explicit Request(void);

  explicit Request(Method _method,
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
  );

  // this should eventually use something like the proposed string_ref instead
  // of std::string
  static Request::Method MethodForStr(std::string const& _s);
  bool isOptions();
  bool isGet();
  bool isHead();
  bool isPost();
  bool isPut();
  bool isDelete();
  bool isTrace();
  bool isConnect();
  bool isPatch();
  bool isUnknown();

  bool hasFormData();
  bool isLocal();
  bool isXhr();

  void check_path_parameters();
  std::string const& ip();
  std::string const& remote_ip();

  std::string const& method();
  std::string const& request_method();

  std::string const& base_url();
  std::string const& original_fullpath();
  std::string const& fullpath();
  std::string const& original_url();
  uint64_t content_length();

  std::string const& authorization();
  std::string const& body();
  std::string const& media_type();
  char const* raw_post();
  std::string const& server_software();

  ~Request();

 private:
  Method original_method_ = Method::UNKNOWN; //constructed

 public:
  std::unordered_map<std::string,std::string> headers;
  std::unordered_map<std::string, std::string> query_parameters;
  std::unordered_map<std::string, std::string> request_parameters;
  dual_map<std::string,std::string> params;

 private:
  std::string const* ip_ = nullptr; //constructed
  std::string const* remote_ip_ = nullptr; //constructed

  std::string const* original_method_str_ = nullptr; //constructed (note: may be nullptr and initialized later, check original_method_)
                                                     //we take ownership of outside strings here
  std::string const* request_method_str_ = nullptr; //if not eqal to original_method_str, it was overridden by someone outside us
                                                    //we take ownership of outside strings here
  std::string const* base_url_ = nullptr; //constructed;

  std::string const* original_fullpath_ = nullptr; //constructed
  std::string const* fullpath_ = nullptr; //constructed (note: normally points to same location as original_fullpath_)
  std::string const* original_url_ = nullptr; //created from base_url_ and original_fullpath_

  uint64_t content_length_ = 0; //constructed
  //std::string const* authorization_; = nullptr; //taken from headers, but created if not there
  std::reference_wrapper<std::string const> authorization_ = empty; //taken from headers, but created if not there
  std::string const* body_ = nullptr; //created from raw_post_
  std::string const* media_type_ = nullptr; //taken from headers, but created if not there
  char const* raw_post_ = nullptr; //constructed
  std::string const* server_software_ = nullptr; //constructed

};

#endif
