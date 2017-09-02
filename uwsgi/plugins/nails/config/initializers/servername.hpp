#include "stdio.h"

#include "nails.h"
#include "filters.h"

bool tackServerInfo(Request&, Response& res) {
  res.headers["Server"] = "nails/NaN";
  return false;
}

bool servername_init() {
  puts("initializing servername");
  nails::response_filters.emplace_back(make_func(&tackServerInfo));
  return true;
}
