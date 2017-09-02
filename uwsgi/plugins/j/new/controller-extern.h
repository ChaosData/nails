#ifndef _CONTROLLER_EXTERN_H_
#define _CONTROLLER_EXTERN_H_

void controller_foo();
void controller_handle_request(char const* path);

enum Method { OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE, CONNECT, PATCH, SPACEJUMP, UNKNOWN };

/*
void handle_request(enum Method _method, char const** headers

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
*/
void init_controllers();
void teardown_controllers();

#endif
