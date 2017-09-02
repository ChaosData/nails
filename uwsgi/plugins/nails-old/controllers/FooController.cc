#include "FooController.h"

FooController::FooController() { }

void FooController::foo(Request& req, Response& res) {
  (void)req;
  (void)res;
  puts("foo");
  puts(req.original_url().c_str());
  if(req.isGet()) {
    puts("is GET!");
  } else {
    puts("not GET");
  }

  if (req.isLocal()) {
    puts("is local!");
  } else {
    puts("not local");
  }

  if (req.isXhr()) {
    puts("is XHR!");
  } else {
    puts("not XHR");
  }

  printf("ip: %s\n", req.ip().c_str());
  printf("remote_ip: %s\n", req.remote_ip().c_str());

  printf("method: %s\n", req.method().c_str());
  printf("request_method: %s\n", req.request_method().c_str());

  printf("base_url: %s\n", req.base_url().c_str());
  printf("original_fullpath: %s\n", req.original_fullpath().c_str());
  printf("fullpath: %s\n", req.fullpath().c_str());
  printf("original_url: %s\n", req.original_url().c_str());
  printf("content_length: %lu\n", req.content_length());

  printf("authorization: %s\n", req.authorization().c_str());
  printf("body: %s\n", req.body().c_str());

  printf("media_type: %s\n", req.media_type().c_str());
  char * __raw_post = (char*)malloc(req.content_length() + 1);
  memcpy(__raw_post, req.raw_post(), req.content_length());
  __raw_post[req.content_length()] = '\0';
  printf("raw_post: %s\n", __raw_post);
  printf("server_software: %s\n", req.server_software().c_str());
  free((void*)__raw_post);
}

void FooController::bar(Request& req, Response& res) {
  (void)req;
  (void)res;
  puts("bar");
}
