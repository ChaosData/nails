#ifndef _NEWCONTROLLER_HPP_
#define _NEWCONTROLLER_HPP_

#include "nails.h"
#include <stdio.h>

class NewController : public BaseController {
 public:

  nails_route<url("/new"), methods("GET")>
  index(Request&) {
    puts("index");
    return "index";
  };

  nails_route<url("/new/<id>"), methods("GET")>
  show(Request&) {
    puts("show");
    return "show";
  };

  nails_route<url("/new"), methods("POST")>
  create(Request&) {
    puts("create");
    return "create";
  };

  nails_route<url("/new/<id>"), methods("DELETE")>
  destroy(Request&) {
    puts("destroy");
    return "destroy";
  };

};

#endif
