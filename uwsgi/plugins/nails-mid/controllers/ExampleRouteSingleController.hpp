#ifndef _EXAMPLEROUTESPLITCONTROLLER_HPP_
#define _EXAMPLEROUTESPLITCONTROLLER_HPP_

#include "nails.h"
#include <stdio.h>

#include <string>
#include <fstream>
#include <streambuf>

class ExampleRouteSingleController : public BaseController {
 public:

  //template<Url url, Methods methods>
  nails_route<url("/custom/<string:id>/destroy"), methods("DELETE", "POST")>
  destroyer(Request& req) {
    (void)req;
    //printf("destroying %s\n", req.rules.at("id").c_str());
    puts("destroy");
    for (auto const& kv : req.rules) {
      printf("%s: %s\n", kv.first.c_str(), kv.second.c_str());
    }
    return "swiggity swooty";
  };


  nails_route<url("/custom"), methods("PUT")>
  putter(Request& req) {
    (void)req;
    //printf("destroying %s\n", req.rules.at("id").c_str());
    puts("putter");
    puts("rules:");
    for (auto const& kv : req.rules) {
      printf("%s: %s\n", kv.first.c_str(), kv.second.c_str());
    }
    puts("====\nargs:");
    for (auto const& kv : req.args) {
      printf("%s: %s\n", kv.first.c_str(), kv.second.c_str());
    }
    puts("====\nforms:");
    for (auto const& kv : req.form) {
      printf("%s: %s\n", kv.first.c_str(), kv.second.c_str());
    }
    puts("====\nfiles:");
    for (auto const& kv : req.files) {
      printf("%s: (%s:%s)\n",
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
    
    return "swiggity swole";
  };


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

#endif
