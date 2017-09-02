#include "controller-extern.h"
#include "common.h"
#include "unistd.h"

#include <unordered_map>
#include <string>
#include <stdlib.h>
#include <memory>

#include "uwsgi-includer.h"
#include "base.h"
#include "http.h"
#include "reqres.h"
#include "router.h"
#include "controller-helper.h"
#include "initializers.h"

//extern struct uwsgi_server uwsgi;

using namespace nails;

int nails_init(){
  uwsgi_log("Initializing >>nails.cc<< plugin\n");
  init_controllers();
  run_initializers();
  return 0;
}


int nails_request(struct wsgi_request *wsgi_req) {
  debug_uwsgi_log(">>nails<<: Got Request!\n");

    // empty request ? // something breaks about this in c++
//  if (!wsgi_req->len) {
//    debug_uwsgi_log( "Invalid request len. skip.\n");
//    return UWSGI_OK;
//  }


  if (uwsgi_parse_vars(wsgi_req)) { //parses headers
    debug_uwsgi_log("Invalid request. skip.\n");
    return UWSGI_OK;
  }


  uint64_t content_length = wsgi_req->post_cl;

  if (content_length > MAX_FILE_CONTENT_LENGTH) {
    return UWSGI_OK;
  }

  std::string content_type{(char const*)wsgi_req->content_type,
                                        wsgi_req->content_type_len};
  
  if (content_length > MAX_NORMAL_CONTENT_LENGTH) {
    if (content_type.find("multipart/form-data; boundary=")
        == std::string::npos) {
      return UWSGI_OK;
    }
  }

  std::unordered_map<std::string,std::string> environ;
  std::unordered_map<std::string,std::string> headers;

  std::string server_name;
  uint16_t server_port = 0;

  std::string remote_addr{(char const*)wsgi_req->remote_addr,
                                       wsgi_req->remote_addr_len};

  std::string query{(char const*)wsgi_req->query_string,
                    wsgi_req->query_string_len};


  std::string method{(char const*)wsgi_req->method, wsgi_req->method_len};

  std::string uri{(char const*)wsgi_req->uri, wsgi_req->uri_len};

  std::string path;
  char const* question = (char const*)memchr(wsgi_req->uri, '?', wsgi_req->uri_len); 
  if (question == nullptr) {
    path.assign(uri);
  } else {
    path.assign((char const*)wsgi_req->uri, question - wsgi_req->uri);
  }

  std::string host{(char const*)wsgi_req->host, wsgi_req->host_len};

  std::string scheme{(char const*)wsgi_req->scheme, wsgi_req->scheme_len};


  for (int i = 0; i < wsgi_req->var_cnt; i += 2) {
    environ.emplace(
      std::make_pair(
        std::string{(char const*)wsgi_req->hvec[i].iov_base,
                                    wsgi_req->hvec[i].iov_len},
        std::string{(char const*)wsgi_req->hvec[i+1].iov_base,
                                    wsgi_req->hvec[i+1].iov_len}
      ) 
    );

    if (wsgi_req->hvec[i].iov_len > 5) {
      if (memcmp("HTTP_", wsgi_req->hvec[i].iov_base, 5) == 0) {
        std::transform((char*)wsgi_req->hvec[i].iov_base + 5,
          (char*)wsgi_req->hvec[i].iov_base
          + wsgi_req->hvec[i].iov_len,
          (char*)wsgi_req->hvec[i].iov_base + 5,
             [](int i) {
               if (i == '_') {
                 return (int)'-';
               } else {
                 return std::tolower(i);
               }
             });

        headers.emplace(
          std::make_pair(
            std::string{ &((char const*)wsgi_req->hvec[i].iov_base)[5],
                                        wsgi_req->hvec[i].iov_len - 5},
            std::string{(char const*)wsgi_req->hvec[i+1].iov_base,
                                     wsgi_req->hvec[i+1].iov_len}
          )
        );
      } else if (wsgi_req->hvec[i].iov_len == 11 &&
                 memcmp("SERVER_NAME", wsgi_req->hvec[i].iov_base, 11) == 0) {
        server_name.assign((char const*)wsgi_req->hvec[i+1].iov_base,
                                        wsgi_req->hvec[i+1].iov_len);
      } else if (wsgi_req->hvec[i].iov_len == 11 &&
                 memcmp("SERVER_PORT", wsgi_req->hvec[i].iov_base, 11) == 0) {
        server_port = strtol((char const*)wsgi_req->hvec[i+1].iov_base, nullptr, 10);
      } else if (wsgi_req->hvec[i].iov_len == sizeof("CONTENT_TYPE")-1 &&
                 memcmp("CONTENT_TYPE", wsgi_req->hvec[i].iov_base,
                                        sizeof("CONTENT_TYPE")-1) == 0) {
        headers.emplace(
          std::make_pair(
            "content-type",
            std::string{(char const*)wsgi_req->hvec[i+1].iov_base,
                                     wsgi_req->hvec[i+1].iov_len
            }
          )
        );
      } else if (wsgi_req->hvec[i].iov_len == sizeof("CONTENT_LENGTH")-1 &&
                 memcmp("CONTENT_LENGTH", wsgi_req->hvec[i].iov_base,
                                        sizeof("CONTENT_LENGTH")-1) == 0) {
        headers.emplace(
          std::make_pair(
            "content-length",
            std::string{(char const*)wsgi_req->hvec[i+1].iov_base,
                                     wsgi_req->hvec[i+1].iov_len
            }
          )
        );
      }
    }
  }

  debug_printf("server_port: %hu\n", server_port);
  debug_printf("server_name: %s\n", server_name.c_str());
  for (auto const& kv : headers) {
    debug_printf("%s: %s\n", kv.first.c_str(), kv.second.c_str());
  }

  for (auto const& kv : environ) {
    debug_printf("%s: %s\n", kv.first.c_str(), kv.second.c_str());
  }


  ParamMap args;

  if (!query.empty()) {
    std::vector<std::string> query_pairs = split(query, '&');

    for (auto const& query_pair : query_pairs) {
      size_t eq_pos = query_pair.find("=");
      if (eq_pos != std::string::npos) {
        args.emplace(std::make_pair(
            decodeURIComponentPlus(query_pair.substr(0,eq_pos)),
            decodeURIComponentPlus(
              query_pair.substr(eq_pos+1, query_pair.size())
            )
          )
        );
      } else {
        args.emplace(std::make_pair(decodeURIComponentPlus(query_pair), ""));
      }
    }
  }

  ParamMap forms;
  FileMap files;

  ParamMap cookies;
  if (headers.find("cookie") != headers.end()) {
    parseCookieHeader(headers["cookie"], cookies);
  }

  char const* body;
  size_t bytes_read = 0;
  if (content_length > 0) {
    if (content_type == "application/x-www-form-urlencoded") {
      debug_puts("decoding");
      parseURLEncodedBody(wsgi_req, forms);
    } else {
      size_t bound = content_type.find("multipart/form-data; boundary=");
      if (bound != 0) {
        body = uwsgi_request_body_read(wsgi_req, wsgi_req->post_cl,
                                             (ssize_t*)&bytes_read);
      } else {

        std::string ctboundary{content_type,
                               sizeof("multipart/form-data; boundary=")-1,
                               std::string::npos};
        bool ret = parseMultipartBody(wsgi_req, ctboundary, forms, files);

        printf("ret: %u\n", ret);

        for (auto const& kv : forms) {
          debug_printf("%s: %s\n", kv.first.c_str(), kv.second.c_str());
        }

        for (auto const& kv : files) {
          debug_printf("%s\n", kv.first.c_str());
          debug_printf("%s: (%s, %s)\n", kv.second.external.filename.c_str(),
                                         kv.second.external.content_type.c_str(),
                                         kv.second.path.c_str());
        }
      }

    }
  }



  Request request{
    std::move(method),
    std::move(headers),
    std::move(cookies),
    std::move(args),
    std::move(forms),
    bytes_read != 0 ? body : nullptr,
    std::move(files),
    std::move(environ),
    "http://127.0.0.1:8088",
    std::string{uri},
    "", //script_root
    std::move(path)    
  };

  if (!route(wsgi_req, request.path, request.method, request)) {
  }

  for (auto const& kv : request.files) {
    remove(kv.second.path.c_str());
  }

  return UWSGI_OK;
}





