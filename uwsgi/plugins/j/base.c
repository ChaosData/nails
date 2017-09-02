#include <uwsgi.h>
#include "controller-extern.h"

extern struct uwsgi_server uwsgi;

int j_init(){
  uwsgi_log("Initializing >>J<< plugin\n");
  init_controllers();
  return 0;
}

int j_request(struct wsgi_request *wsgi_req) {
  uwsgi_log(">>J<<: Got Request!\n");

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
  printf("scriptName(%lu): %s\n", wsgi_req->script_name_len, wsgi_req->script_name);
  printf("pathInfo(%lu): %s\n", wsgi_req->path_info_len, wsgi_req->path_info);
  printf("queryString(%lu): %s\n", wsgi_req->query_string_len, wsgi_req->query_string);
  printf("host(%lu): %s\n", wsgi_req->host_len, wsgi_req->host);
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
  printf("wsgi_req->var_cnt: %lu\n", wsgi_req->var_cnt);

  for (int i = 0; i < wsgi_req->var_cnt; i += 2) {
    uwsgi_debug("%.*s: %.*s\n", wsgi_req->hvec[i].iov_len, wsgi_req->hvec[i].iov_base, wsgi_req->hvec[i+1].iov_len, wsgi_req->hvec[i+1].iov_base);
  }

  ssize_t bytes_read = 0;
  char* body = uwsgi_request_body_read(wsgi_req, wsgi_req->post_cl, &bytes_read);
  printf("body(%ld): %s\n", bytes_read, body);


  uint16_t req_path_len;
  char* req_path = (char*)uwsgi_get_var(wsgi_req, (char *) "PATH_INFO", 9, &req_path_len);

  controller_handle_request(&(req_path[1]));

  uwsgi_response_prepare_headers(wsgi_req, (char *)"200 OK", 6);
  uwsgi_response_add_content_type(wsgi_req, (char *)"text/html", 9);
  uwsgi_response_write_body_do(wsgi_req, req_path, req_path_len);

clear:
  return UWSGI_OK;
}





