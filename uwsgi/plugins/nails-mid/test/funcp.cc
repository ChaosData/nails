#include <stdio.h>
#include <stdint.h>

int tester(void*, char const*, uint16_t) {
  puts("gg");
  return 0;
}

int (*uwsgi_response_prepare_headers) (void*, char const *, uint16_t) = tester;

int main() {
  uwsgi_response_prepare_headers(nullptr, nullptr, 0);
  return 0;
}

