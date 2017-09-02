#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>
#include <unordered_map>
#include <stdexcept>
#include <iostream>

#include "common.h"
#include "controller-helper.h"
#include "router.h"

namespace nails {

Router mock_router{};

void print_map(std::unordered_map<std::string,std::string> const& map) {
  for (size_t i = 0; i < map.bucket_count(); ++i) {
    printf("bucket #%lu contains:\n", i);
    for (auto local_it = map.begin(i); local_it!= map.end(i); ++local_it ) {
      printf("%s: %s\n", local_it->first.c_str(), local_it->second.c_str());
    }
  }
}

// void dyncall(char const* _cname, char const* mname) {
  
//   // printf("_cname: %s\n", _cname);
//   // printf("mname: %s\n", mname);
//   // printf("controller_mapper: %lu\n", controller_mapper.size());
//   // printf("controller_name_mapper: %lu\n", controller_name_mapper.size());

//   // for (size_t i = 0; i < controller_mapper.bucket_count(); ++i) {
//   //   printf("bucket #%lu contains:\n", i);
//   //   for (auto local_it = controller_mapper.begin(i); local_it!= controller_mapper.end(i); ++local_it ) {
//   //     printf("%s\n", local_it->first);
//   //   }
//   // }

//   // for (size_t i = 0; i < controller_name_mapper.bucket_count(); ++i) {
//   //   printf("bucket #%lu contains:\n", i);
//   //   for (auto local_it = controller_name_mapper.begin(i); local_it!= controller_name_mapper.end(i); ++local_it ) {
//   //     printf("%s: %s\n", local_it->first.c_str(), local_it->second);
//   //   }
//   // }
  
//   char const* cname = _cname;
//   ControllerActionMapper* cam;
//   try {
//     cam = &(controller_mapper.at(cname));
//   } catch (std::out_of_range oor) {
//     try {
//       cname = controller_name_mapper.at(std::string(cname));
//       cam = &(controller_mapper.at(cname));
//     } catch (std::out_of_range oor) {
//       puts("Couldn't find the controller!");
//       return;
//     }
//   }
//   auto* f0 = static_cast<func<void(BaseController*,Request&,Response&)>*>(cam->get(mname));
//   if (f0 == nullptr) {
//     puts("null");
//   } else {
//     auto req = Request();
//     auto res = Response();

//     BaseController* bcp = controller_factory_mapper.at(cname)->create();
//     if (bcp == nullptr) {
//       puts("bcp null");
//     } else {
//       (*f0)(std::move(bcp), req, res);
//       delete bcp;
//     }
//   }
// }

//controller-extern.h

/*
void controller_handle_request(char const* path) {
  std::string p{path};
  p = p.substr(0, p.size()-1);

  std::vector<std::string> parts = split(p, '/');
  if (parts.size() == 1) {
    dyncall(parts[0].c_str(), "index");
  } else if (parts.size() == 2) {
    printf(">>%s<<\n", parts[0].c_str());
    printf(">>%s<<\n", parts[1].c_str());
    dyncall(parts[0].c_str(), parts[1].c_str());
  } else {
    printf("<?> Can't handle /%s", path);
  }

}
*/


int8_t get_addr_str(char const* hostname, char* dst) {
  struct addrinfo* result;

  int error = getaddrinfo(hostname, NULL, NULL, &result);
  if (error != 0) {
    if (error == EAI_SYSTEM) {
      return -1;
    } else {
      return -2;
    }
    return -3;
  }
  switch (result->ai_family) {
    case AF_INET: {
      inet_ntop(AF_INET, &( ((struct sockaddr_in*)(result->ai_addr))->sin_addr ), dst, INET_ADDRSTRLEN);
      break;
    }
    case AF_INET6:
      inet_ntop(AF_INET6, &( ((struct sockaddr_in*)(result->ai_addr))->sin_addr ), dst, INET6_ADDRSTRLEN);
      break;
    default:
      return -4;
      break;
  }
  freeaddrinfo(result);
  return 1;
}



void dyncall_new(char const* _cname, char const* mname, Request& req) {
  char const* cname = _cname;
  ControllerActionMapper* cam;
  try {
    cam = &(controller_mapper.at(cname));
  } catch (std::out_of_range oor) {
    try {
      cname = controller_name_mapper.at(std::string(cname));
      cam = &(controller_mapper.at(cname));
    } catch (std::out_of_range oor) {
      puts("Couldn't find the controller!");
      return;
    }
  }
  auto* f0 = static_cast<func<Response(BaseController*,Request&)>*>(cam->get(mname));
  if (f0 == nullptr) {
    puts("null");
  } else {
    BaseController* bcp = controller_factory_mapper.at(cname)->create();
    if (bcp == nullptr) {
      puts("bcp null");
    } else {
      (*f0)(std::move(bcp), req);
      delete bcp;
    }
  }
}



void handle_request(struct wsgi_request* wsgi_req, raw_request const* req) {
  puts(">>handle_request<<");
  std::unordered_map<std::string,std::string> headers;
  raw_header_t const* raw_headers = req->headers;
  for (size_t i(0); i < req->headers_count; i++) {
    std::string header_name{raw_headers[i].name,raw_headers[i].name_len};
    std::transform(header_name.begin(), header_name.end(), header_name.begin(),
                   [](int i) {
                     if (i == '_') {
                       return (int)'-';
                     } else {
                       return std::tolower(i);
                     }
                   });
    headers.emplace(std::make_pair(std::move(header_name),
                                   std::string(raw_headers[i].value,
                                   raw_headers[i].value_len)
                   )
    );
  }
  headers.emplace(std::make_pair("content-type",
                                 std::string(req->content_type,
                                 req->content_type_len))
  );
  //print_map(headers);

  puts(std::string(req->query, req->query_len).c_str());  

  std::unordered_map<std::string,std::string> query_params;
  std::vector<std::string> query_pairs =
      split(std::string(req->query, req->query_len), '&');

  for (auto const& query_pair : query_pairs) {
    size_t eq_pos = query_pair.find("=");
    query_params.emplace(std::make_pair(query_pair.substr(0,eq_pos),
                                        query_pair.substr(eq_pos+1,query_pair.size()))
    );
  }
  //print_map(query_params);

  std::unordered_map<std::string,std::string> request_params;
  //TODO: fill that in for www-urlencoded and multi-part
  //      (hold off on the latter for now)

  std::string server_name{req->server_name, req->server_name_len};
  char server_ip_str[INET6_ADDRSTRLEN] = {0};
  int8_t result = get_addr_str(server_name.c_str(), server_ip_str);
  if (result == 1) {
    //puts(server_ip_str);
  }

  
  // std::string* ip = new std::string(server_ip_str);
  // std::string* remote_ip = new std::string(req->remote_addr, req->remote_addr_len);
  std::string method(req->method, req->method_len);
  // std::string* request_method_s = method_s;
  // Request::Method method{Request::MethodForStr(*method_s)};
  // std::string* base_url = new std::string();
  // base_url->reserve(req->scheme_len + req->server_name_len);
  // base_url->append(req->scheme, req->scheme_len);
  // try {
  //   base_url->append(headers.at("host"));
  // } catch (std::out_of_range) {
  //   base_url->append(server_name);
  // }
  std::string fullpath(req->uri, req->uri_len);
  // std::string* original_fullpath = fullpath;
  // std::string* server_software = new std::string("uwsgi/Nails");
  
/*  Request request{method, std::move(headers), std::move(query_params),
      std::move(request_params), ip, remote_ip, method_s, request_method_s,
      base_url, original_fullpath, fullpath, req->content_length,
      req->body, server_software};*/

// Request::Request(std::string&& _method,
//                  std::unordered_map<std::string,std::string>&& _headers,
//                  std::unordered_map<std::string, std::string>&& _args,
//                  std::unordered_map<std::string, std::string>&& _form,
//                  char const* _stream,
//                  std::unordered_map<std::string, FileStorage>&& _files,
//                  std::unordered_map<std::string, std::string>&& _environ,
//                  std::string&& _origin,
//                  std::string&& _request_uri, //sans script_root
//                  std::string&& _script_root,
//                  std::string&& _path) 

  Request request{
    std::string(req->method, req->method_len),
    std::move(headers),
    std::move(query_params),
    std::move(request_params),
    req->body_len != 0 ? req->body : nullptr,
    std::unordered_map<std::string, FileStorage>{},
    std::unordered_map<std::string, std::string>{},
    "http://127.0.0.1:8088",
    std::string(req->uri, req->uri_len),
    "", //script_root
    std::string(req->path, req->path_len)       
  };

  // if (!mock_router.route(std::string(req->path, req->path_len), *method_s, request, response)) {
  //   std::string p{req->path, req->path_len};
  //   p = p.substr(1, p.size());

  //   std::vector<std::string> parts = split(p, '/');
  //   if (parts.size() == 1) {
  //     dyncall_new(parts[0].c_str(), "index", request, response);
  //   } else if (parts.size() == 2) {
  //     dyncall_new(parts[0].c_str(), parts[1].c_str(), request, response);
  //   } else {
  //     printf("<?> Can't handle /%s\n", fullpath->c_str());
  //   }
  //}
  puts(std::string(req->path, req->path_len).c_str());
  if (!route(wsgi_req, std::string(req->path, req->path_len), method,
                    request)) {
    std::string p{req->path, req->path_len};
    p = p.substr(1, p.size());

    std::vector<std::string> parts = split(p, '/');
    if (parts.size() == 1) {
      dyncall_new(parts[0].c_str(), "index", request);
    } else if (parts.size() == 2) {
      dyncall_new(parts[0].c_str(), parts[1].c_str(), request);
    } else {
      printf("[??] Can't handle %s\n", fullpath.c_str());
    }
  }

}


};
