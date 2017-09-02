#include "stdio.h"

#include "nails.h"
#include "filters.h"

class MethodOverrideRequest : private Request {
 public:
  MethodOverrideRequest() = delete;
  bool override_method() {
    std::string const& m = form_["_method"];
    puts(m.c_str());
    if (m == "GET") {
      return true; //kill request
    }
    method_ = form_["_method"];
    return false;
  }
};

bool method_override(Request& req) {
  if (req.method == "POST" && req.form.find("_method") != req.form.end()) {
    return ((MethodOverrideRequest&)req).override_method();
  }
  return false;
}

Response override_illegal_response(Request&) {
  puts("illegal");
  return make_response("Illegal override.", 403, {});
}

bool method_override_init() {
  puts("initializing method override");
  nails::prematch_filters.emplace_back(
    std::make_pair(
      make_func(&method_override),
      make_func(&override_illegal_response)
    )
  );

  return true;
}
