#include "stdio.h"

#include "nails.h"
#include "filters.h"

std::set<std::string> const cors_whitelist = {
  "http://127.0.0.1", "https://127.0.0.1",
  "http://localhost", "https://localhost"
};

bool cors_filter(Request& req) {
  if (req.method == "OPTIONS" &&
        (
          req.headers.find("access-control-request-method")
          != req.headers.end()
        || req.headers.find("access-control-request-headers")
           != req.headers.end()
        )
     ) {
    return true;
  }
  return false;
}

Response preflight_response(Request& req) {
  if (req.headers.find("origin") == req.headers.end()) {
    return make_response("", 403, {});
  }

  return make_response("", 200, {
    {"Access-Control-Allow-Methods", "HEAD, GET, OPTIONS, "
                                     "POST, PUT, PATCH, DELETE"},
    {"Access-Control-Allow-Headers", "X-Requested-With, X-CSRF-Token, "
                                     "Content-Type, Authorization"},
    {"Access-Control-Allow-Credentials", "true"},
    {"Access-Control-Allow-Origin", req.headers.at("origin")},
    {"Access-Control-Max-Age", "21600"}
  });
}

bool cors_response(Request& req, Response& res) {
  if (req.headers.find("origin") == req.headers.end()) {
    return false;
  }

  std::string wild = "*"; //<form>-equivalent
  std::string origin = req.headers.at("origin");

  //This prevents preflight_response's one from being stomped over and also
  //allows request handlers to maintain locally scoped whitelists.
  if (res.headers.find("Access-Control-Allow-Origin") == res.headers.end()) {
    res.headers["Access-Control-Allow-Origin"] =
        (cors_whitelist.find(origin) != cors_whitelist.end())
        ? origin : wild;

  }

  return false;
}

bool cors_init() {

  puts("initializing CORS filter");
  nails::prematch_filters.emplace_back(
    std::make_pair(
      make_func(&cors_filter),
      make_func(&preflight_response)
    )
  );

  nails::response_filters.emplace_back(
    make_func(&cors_response)
  );

  

  return true;
}
