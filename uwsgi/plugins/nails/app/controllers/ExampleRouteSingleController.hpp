#ifndef _EXAMPLEROUTESPLITCONTROLLER_HPP_
#define _EXAMPLEROUTESPLITCONTROLLER_HPP_

#include "nails.h"
#include <stdio.h>

#include <string>
#include <fstream>
#include <streambuf>

class ExampleRouteSingleController : public BaseController {
 public:

  ExampleRouteSingleController() {
    //protect_from_forgery = false;
  }

  nails_route<url("/custom/<string:id>/destroy"), methods("DELETE", "POST")>
  destroyer(Request& req) {
    (void)req;
    printf("destroy: %s\n", req.rules.at("id").c_str());
    for (auto const& kv : req.rules) {
      printf("%s: %s\n", kv.first.c_str(), kv.second.c_str());
    }
    return "swiggity swooty";
  };


  nails_route<url("/custom"), methods("PUT")>
  putter(Request& req) {
    puts("putter");
    puts("headers:");
    for (auto const& kv : req.headers) {
      printf("%s: %s\n", kv.first.c_str(), kv.second.c_str());
    }
    puts("========");
    
    puts("cookies:");
    for (auto const& kv : req.cookies) {
      printf("  %s: %s\n", kv.first.c_str(), kv.second.c_str());
    }
    puts("========");

    puts("environ:");
    for (auto const& kv : req.environ) {
      printf("  %s: %s\n", kv.first.c_str(), kv.second.c_str());
    }
    puts("========");


    puts("rules:");
    for (auto const& kv : req.rules) {
      printf("  %s: %s\n", kv.first.c_str(), kv.second.c_str());
    }
    puts("========");

    puts("args:");
    for (auto const& kv : req.args) {
      printf("  %s: %s\n", kv.first.c_str(), kv.second.c_str());
    }
    puts("========");

    puts("forms:");
    for (auto const& kv : req.form) {
      printf("  %s: %s\n", kv.first.c_str(), kv.second.c_str());
    }
    puts("========");

    puts("files:");
    for (auto const& kv : req.files) {
      printf("  %s: (%s:%s)\n",
             kv.first.c_str(),
             kv.second.external.filename.c_str(),
             kv.second.path.c_str()
      );
      std::ifstream t(kv.second.path);
      std::string str;

      t.seekg(0, std::ios::end);   
      str.reserve(t.tellg());
      t.seekg(0, std::ios::beg);

      str.assign((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());

      puts(str.c_str());
    }
    puts("========");
    
    Response res = make_response("\"swiggity swole\"", 200, {{"Content-Type","application/json"}});
    res.set_cookie("foo", "bar", 30);
    return res;
  };



  nails_route<url("/custom"), methods("GET")>
  getter(Request& req) {
    puts("getter");
    for (auto const& kv : req.headers) {
      printf("%s: %s\n", kv.first.c_str(), kv.second.c_str());
    }
    if (req.headers.find("cookie") != req.headers.end()) {
      return make_response("{\"authed\": true}\n", 200, {
        {"Content-Type","application/json"},
        {"Access-Control-Allow-Origin", "https://isecpartners.github.io"},
        {"Access-Control-Allow-Credentials", "true"}
      });
    } else {
      return make_response("{\"authed\": false}\n", 200, {
        {"Content-Type","application/json"}
      });
    }
  }


  nails_route<url("/download"), methods("GET")>
  download(Request& req) {
    puts("download(Request& req)");

    auto file = req.args.find("file");
    if (file == req.args.end()) {
      return make_response("", 404, {});
    }
    puts(file->second.c_str());
    std::string filepath = std::string{"./"} + file->second;
    char const* real = realpath(filepath.c_str(), nullptr);
    if (real == nullptr) {
      return make_response("", 404, {});
    }
    filepath.assign(real);

    char const* current = realpath("./", nullptr);
    bool ok = startsWith(real, current);
    free((void*)current);
    free((void*)real);

    if (ok) {
      auto segments = split(filepath, '/');
      if (!segments.empty()) {
        return send_file(filepath, "application/binary",
                         true, segments[segments.size()-1]);
      }
    }
    return make_response("", 403, {});
  }

  nails_route<url("/cookie_test"), methods("GET")>
  cookie_test(Request& req) {
    puts(">>cookie_test<<");
    std::string test;
    if (mapHas(req.session, "test")) {
      test = req.session["test"];
    } else {
      test = "<not found>";
    }
    printf("req.session[\"test\"](%lu): %s\n", test.size(), test.c_str());
    req.session["test"] = "hello world";
    puts("<<cookie_test>>");
    return R"str(<!doctype html>
<html>
  <head>
  </head>
  <body>
    <h1>Test</h1>
  </body>
</html>
)str";
  }

  Response fooer(Request& req) {
    (void)req;
    puts("fooer");
    return "fooer";
  }

  Response fooer2(Request& req) {
    (void)req;
    puts("fooer2");
    return "fooer2";
  }

 private:
  Response fooer3(Request& req) {
    (void)req;
    puts("fooer3");
    return "fooer3";
  }

};


class ChildExampleRouteSingleController : public ExampleRouteSingleController {
 public:

  nails_route<url("/custom/child/<string:id>"), methods("DELETE")>
  destroyer(Request& req) {
    (void)req;
    return "swiggity swild";
  };

  Response child(Request& req) {
    (void)req;
    return "child";
  }
 private:
  nails_route<url("/custom/child/<string:id>/2"), methods("DELETE")>
  destroyer2(Request& req) {
    (void)req;
    return "swiggity swild2";
  };
};
#endif
