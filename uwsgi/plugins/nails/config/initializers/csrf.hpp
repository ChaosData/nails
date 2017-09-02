#include "nails.h"
#include "filters.h"
#include "sodium.h"
#include "common.h"

#include <string>


//checks whether or not a CSRF token is needed and then checks the token
std::set<std::string> csrf_method_whitelist{"HEAD", "GET", "OPTIONS"};
bool csrf_filter(Request& req) {
  if (csrf_method_whitelist.find(req.method) != csrf_method_whitelist.end()) {
    return false;
  }

  nails::factory_base* factory = nullptr;
  nails::func_base* action = nullptr;
  std::tie(factory, action) = req.endpoint;

  if (factory == nullptr || action == nullptr
  || nails::controller_factory_singleton_mapper.at(factory)->protect_from_forgery) {
    if(mapHas(req.form, "csrf_token")) {
      if (mapHas(req.session, "csrf_token")) {
        std::string const& form_token = req.form.at("csrf_token");
        std::string const& csrf_token = req.get_csrf_token();
        if (form_token.size() == csrf_token.size()) {
          if (sodium_memcmp((void*)form_token.data(),
                            (void*)csrf_token.data(),
                            csrf_token.size()) == 0) {
            return false;
          }
        }
      }
    }
  } else {
    return false;
  }
  return true;
}

//error response if failed CSRF check
Response csrf_failed_response(Request&) {
  return make_response("", 422, {});
}

//used to obtain or 
bool csrf_token_filter(Request& req) {
  if (req.path == "/csrf_token") {
    if (req.method == "GET") {
      return true;
    } else if (req.method == "POST") {
      // a reset mechanism
      // it should be CSRF validated by the above filter
      req.reset_csrf_token();
      return true;
    }
  }
  return false;
}

Response csrf_token_hook(Request& req) {
  json res_obj;
  std::string const& ct = req.get_csrf_token();
  res_obj["token"] = req.get_csrf_token();
  return make_response(res_obj, 200, {});
}

bool csrf_init() {
  puts("initializing csrf filter");
  nails::postmatch_filters.emplace_back(
    std::make_pair(
      make_func(&csrf_filter),
      make_func(&csrf_failed_response)
    )
  );
  nails::postmatch_filters.emplace_back(
    std::make_pair(
      make_func(&csrf_token_filter),
      make_func(&csrf_token_hook)
    )
  );
  return true;
}
