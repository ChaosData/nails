#ifndef _CONTROLLER_EXTERN_H_
#define _CONTROLLER_EXTERN_H_

#include <stdint.h>



typedef struct raw_header {
  char const* name; uint32_t name_len;
  char const* value; uint32_t value_len;
} raw_header_t;

typedef struct raw_request {
  char const* method; uint8_t method_len;
  char const* uri; uint32_t uri_len;
  char const* path; uint32_t path_len;
  char const* host; uint32_t host_len;
  char const* server_name; uint32_t server_name_len;
  uint16_t    server_port;
  char const* scheme; uint32_t scheme_len;
  char const* remote_addr; uint32_t remote_addr_len;
  uint16_t    remote_port;
  char const* query; uint32_t query_len;
  char const* content_type; uint32_t content_type_len;
  uint64_t    content_length;
  char const* body; uint32_t body_len;
  raw_header_t const* headers; uint16_t headers_count;
} raw_request_t;


#ifdef __cplusplus
extern "C" {
#endif
  void init_controllers();
  void teardown_controllers();

  void controller_foo();

  void controller_handle_request(char const* path);
  void handle_request(raw_request_t const* req);
#ifdef __cplusplus
}
#endif



#endif
