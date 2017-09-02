#include <uwsgi.h>
#include "controller-extern.h"

extern struct uwsgi_server uwsgi;

//#define NAILS_DEBUG 1

#ifdef NAILS_DEBUG
  #define debug_printf(...) printf(__VA_ARGS__)
  #define debug_puts(...) puts(__VA_ARGS__)
  #define debug_uwsgi_log(...) uwsgi_log(__VA_ARGS__)
#else
  #define debug_printf(...) (void)0
  #define debug_puts(...) (void)0
  #define debug_uwsgi_log(...) (void)0
#endif


int nails_init(){
  uwsgi_log("Initializing >>nails<< plugin\n");
  init_controllers();
  return 0;
}

int nails_request(struct wsgi_request *wsgi_req) {
  debug_uwsgi_log(">>nails<<: Got Request!\n");

  // empty request ?
  if (!wsgi_req->len) {
    debug_uwsgi_log( "Invalid request. skip.\n");
    goto clear;
  }

  if (uwsgi_parse_vars(wsgi_req)) {
    debug_uwsgi_log("Invalid request. skip.\n");
    goto clear;
  }

  debug_printf("method(%lu): %s\n", wsgi_req->method_len, wsgi_req->method);
  debug_printf("uri(%lu): %s\n", wsgi_req->uri_len, wsgi_req->uri);
  debug_printf("scriptName(%lu): %s\n", wsgi_req->script_name_len, wsgi_req->script_name);
  debug_printf("pathInfo(%lu): %s\n", wsgi_req->path_info_len, wsgi_req->path_info);
  debug_printf("queryString(%lu): %s\n", wsgi_req->query_string_len, wsgi_req->query_string);
  debug_printf("host(%lu): %s\n", wsgi_req->host_len, wsgi_req->host);
  debug_printf("protocol(%lu): %s\n", wsgi_req->protocol_len, wsgi_req->protocol);
  debug_printf("cookie(%lu): %s\n", wsgi_req->cookie_len, wsgi_req->cookie);
  if (wsgi_req->scheme_len) {
    //takes X-Forwarded-Proto when running in http mode
    //that seems dumb
    debug_printf("scheme(%lu): %s\n", wsgi_req->scheme_len, wsgi_req->scheme);
  } else {
    debug_puts("using http://");
  }
  debug_printf("remoteAddr(%lu): %s\n", wsgi_req->remote_addr_len, wsgi_req->remote_addr);

  if (wsgi_req->remote_user_len) {
    debug_printf("remoteUser(%lu): %s\n", wsgi_req->remote_user_len, wsgi_req->remote_user);
  } else {
    debug_puts("no remote user");
  }
  debug_printf("Content-Length: %lu\n", wsgi_req->post_cl);
  debug_printf("Content-Type(%lu): %s\n", wsgi_req->content_type_len, wsgi_req->content_type);
  debug_printf("wsgi_req->var_cnt: %hu\n", wsgi_req->var_cnt);
  debug_printf("wsgi_req->header_cnt: %hu\n", wsgi_req->header_cnt);

  uint16_t headers_count = 0;

  for (int i = 0; i < wsgi_req->var_cnt; i += 2) {
    if (wsgi_req->hvec[i].iov_len > 5) {
      if (memcmp("HTTP_", wsgi_req->hvec[i].iov_base, 5) == 0) {
        headers_count += 1;
      }
    }
  }
  debug_printf("headers_count: %hu\n", headers_count);


  for (int i = 0; i < wsgi_req->var_cnt; i += 2) {
    debug_uwsgi_log("%.*s: %.*s\n", wsgi_req->hvec[i].iov_len, wsgi_req->hvec[i].iov_base, wsgi_req->hvec[i+1].iov_len, wsgi_req->hvec[i+1].iov_base);
  }
  debug_puts(">>>>>>>>>");
  ssize_t bytes_read = 0;
  char* body = uwsgi_request_body_read(wsgi_req, wsgi_req->post_cl, &bytes_read);
  char* __body = (char*)malloc(bytes_read+1);
  memcpy(__body, body, bytes_read);
  __body[bytes_read] = '\0';
  debug_puts("<<<<<<<<<");
  
  debug_printf("body(%ld): %s\n", bytes_read, __body);
  free(__body);
  debug_puts(">>>>>>>>>");


  //uint16_t req_path_len;
  //char* req_path = (char*)uwsgi_get_var(wsgi_req, (char *) "PATH_INFO", 9, &req_path_len);

  //controller_handle_request(&(req_path[1]));

  raw_request_t req;
  req.headers_count = headers_count;
  req.headers = NULL;
  req.headers_count = headers_count;
  raw_header_t* headers =calloc(headers_count, sizeof(raw_header_t));

  req.method = wsgi_req->method; req.method_len = wsgi_req->method_len;
  req.uri = wsgi_req->uri; req.uri_len = wsgi_req->uri_len;

  //path_info decodes url encodings in the path, need to parse request_uri for path
  //req.path = wsgi_req->path_info; req.path_len = wsgi_req->path_info_len;
  char const* question = memchr(req.uri, '?', req.uri_len); 
  req.path = req.uri;
  if (question == NULL) {
    req.path_len = req.uri_len;
  } else {
    req.path_len = question - req.path;
  }
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


  handle_request(wsgi_req, &req);


  free(headers);
clear:
  return UWSGI_OK;
}





