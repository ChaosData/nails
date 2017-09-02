#include "initializers.h"

#include "initializers/servername.hpp"
#include "initializers/method_override.hpp"
#include "initializers/cors.hpp"
#include "initializers/sessions.hpp"
#include "initializers/csrf.hpp"

void run_initializers() {
  servername_init();
  method_override_init();
  cors_init();
  sessions_init();
  csrf_init();
}