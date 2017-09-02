#ifndef _CONTROLLER_EXTERN_H_
#define _CONTROLLER_EXTERN_H_

void controller_foo();
void controller_handle_request(char const* path);

void init_controllers();
void teardown_controllers();

#endif
