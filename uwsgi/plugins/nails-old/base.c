#include <uwsgi.h>
#include "controller-extern.h"

extern struct uwsgi_server uwsgi;

int nails_init(){
  uwsgi_log("Initializing >>nails<< plugin\n");
  init_controllers();
  return 0;
}

int nails_request(struct wsgi_request *wsgi_req) {
  uwsgi_log(">>nails<<: Got Request!\n");

  // empty request ?
  if (!wsgi_req->len) {
    uwsgi_log( "Invalid request. skip.\n");
    goto clear;
  }

  if (uwsgi_parse_vars(wsgi_req)) {
    uwsgi_log("Invalid request. skip.\n");
    goto clear;
  }

  printf("method(%lu): %s\n", wsgi_req->method_len, wsgi_req->method);
  printf("uri(%lu): %s\n", wsgi_req->uri_len, wsgi_req->uri);
  printf("scriptName(%lu): %s\n", wsgi_req->script_name_len, wsgi_req->script_name);
  printf("pathInfo(%lu): %s\n", wsgi_req->path_info_len, wsgi_req->path_info);
  printf("queryString(%lu): %s\n", wsgi_req->query_string_len, wsgi_req->query_string);
  printf("host(%lu): %s\n", wsgi_req->host_len, wsgi_req->host);
  printf("protocol(%lu): %s\n", wsgi_req->protocol_len, wsgi_req->protocol);
  printf("cookie(%lu): %s\n", wsgi_req->cookie_len, wsgi_req->cookie);
  if (wsgi_req->scheme_len) {
    //takes X-Forwarded-Proto when running in http mode
    //that seems dumb
    printf("scheme(%lu): %s\n", wsgi_req->scheme_len, wsgi_req->scheme);
  } else {
    puts("using http://");
  }
  printf("remoteAddr(%lu): %s\n", wsgi_req->remote_addr_len, wsgi_req->remote_addr);

  if (wsgi_req->remote_user_len) {
    printf("remoteUser(%lu): %s\n", wsgi_req->remote_user_len, wsgi_req->remote_user);
  } else {
    puts("no remote user");
  }
  printf("Content-Length: %lu\n", wsgi_req->post_cl);
  printf("Content-Type(%lu): %s\n", wsgi_req->content_type_len, wsgi_req->content_type);
  printf("wsgi_req->var_cnt: %hu\n", wsgi_req->var_cnt);
  printf("wsgi_req->header_cnt: %hu\n", wsgi_req->header_cnt);

  uint16_t headers_count = 0;

  for (int i = 0; i < wsgi_req->var_cnt; i += 2) {
    if (wsgi_req->hvec[i].iov_len > 5) {
      if (memcmp("HTTP_", wsgi_req->hvec[i].iov_base, 5) == 0) {
        headers_count += 1;
      }
    }
  }
  printf("headers_count: %hu\n", headers_count);


  for (int i = 0; i < wsgi_req->var_cnt; i += 2) {
    uwsgi_debug("%.*s: %.*s\n", wsgi_req->hvec[i].iov_len, wsgi_req->hvec[i].iov_base, wsgi_req->hvec[i+1].iov_len, wsgi_req->hvec[i+1].iov_base);
  }
  puts(">>>>>>>>>");
  ssize_t bytes_read = 0;
  char* body = uwsgi_request_body_read(wsgi_req, wsgi_req->post_cl, &bytes_read);
  char* __body = (char*)malloc(bytes_read+1);
  memcpy(__body, body, bytes_read);
  __body[bytes_read] = '\0';
  puts("<<<<<<<<<");
  
  printf("body(%ld): %s\n", bytes_read, __body);
  free(__body);
  puts(">>>>>>>>>");


  uint16_t req_path_len;
  char* req_path = (char*)uwsgi_get_var(wsgi_req, (char *) "PATH_INFO", 9, &req_path_len);

  //controller_handle_request(&(req_path[1]));

  raw_request_t req;
  req.headers_count = headers_count;
  req.headers = NULL;
  req.headers_count = headers_count;
  raw_header_t* headers =calloc(headers_count, sizeof(raw_header_t));

  req.method = wsgi_req->method; req.method_len = wsgi_req->method_len;
  req.uri = wsgi_req->uri; req.uri_len = wsgi_req->uri_len;
  req.path = wsgi_req->path_info; req.path_len = wsgi_req->path_info_len;
  req.host = wsgi_req->host; req.host_len = wsgi_req->host_len;
  req.server_name = NULL; req.server_name_len = 0;
  req.server_port = 0;
  req.scheme = wsgi_req->scheme; req.scheme_len = wsgi_req->scheme_len;
  req.remote_addr = wsgi_req->remote_addr; req.remote_addr_len = wsgi_req->remote_addr_len;
  req.query = wsgi_req->query_string; req.query_len = wsgi_req->query_string_len;
  req.content_type = wsgi_req->content_type; req.content_type_len = wsgi_req->content_type_len;
  req.content_length = wsgi_req->post_cl;
  req.body = body; req.body_len = bytes_read;

  uint16_t headers_pos = 0;
  for (int i = 0; i < wsgi_req->var_cnt; i += 2) {
    if (wsgi_req->hvec[i].iov_len > 5) {
      if (memcmp("HTTP_", wsgi_req->hvec[i].iov_base, 5) == 0) {
        headers[headers_pos].name = &(((char*)(wsgi_req->hvec[i].iov_base))[5]);
        headers[headers_pos].name_len = wsgi_req->hvec[i].iov_len - 5;
        headers[headers_pos].value = wsgi_req->hvec[i+1].iov_base;
        headers[headers_pos].value_len = wsgi_req->hvec[i+1].iov_len;
        headers_pos += 1;
        if (headers_pos == headers_count) {
          break;
        }
      } else if (wsgi_req->hvec[i].iov_len == 11 &&
                 memcmp("SERVER_NAME", wsgi_req->hvec[i].iov_base, 11) == 0) {
        req.server_name = wsgi_req->hvec[i+1].iov_base;
        req.server_name_len = wsgi_req->hvec[i+1].iov_len;
      } else if (wsgi_req->hvec[i].iov_len == 11 &&
                 memcmp("SERVER_PORT", wsgi_req->hvec[i].iov_base, 11) == 0) {
        char port[6] = {0};
        if (wsgi_req->hvec[i+1].iov_len <= 5) {
          memcpy(port, wsgi_req->hvec[i+1].iov_base, wsgi_req->hvec[i+1].iov_len);
          req.server_port = atoi(port);
        }
      }
    }
  }
  req.headers = headers;


/*
  char const* body; uint32_t body_len;
  raw_header_t const* headers; uint16_t headers_count;
} raw_request_t;
*/
  handle_request(&req);


  uwsgi_response_prepare_headers(wsgi_req, (char *)"200 OK", 6);
  uwsgi_response_add_content_type(wsgi_req, (char *)"text/html", 9);
  uwsgi_response_write_body_do(wsgi_req, req_path, req_path_len);
  free(headers);
clear:
  return UWSGI_OK;
}





