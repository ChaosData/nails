#include <string>
#include <unordered_map>
#include <stdexcept>

#include <stdio.h>
#include <iostream>

#include "controller-helper.h"

void dyncall(char const* _cname, char const* mname) {
  /*
  printf("_cname: %s\n", _cname);
  printf("mname: %s\n", mname);
  printf("controller_mapper: %lu\n", controller_mapper.size());
  printf("controller_name_mapper: %lu\n", controller_name_mapper.size());

  for (size_t i = 0; i < controller_mapper.bucket_count(); ++i) {
    printf("bucket #%lu contains:\n", i);
    for (auto local_it = controller_mapper.begin(i); local_it!= controller_mapper.end(i); ++local_it ) {
      printf("%s\n", local_it->first);
    }
  }

  for (size_t i = 0; i < controller_name_mapper.bucket_count(); ++i) {
    printf("bucket #%lu contains:\n", i);
    for (auto local_it = controller_name_mapper.begin(i); local_it!= controller_name_mapper.end(i); ++local_it ) {
      printf("%s: %s\n", local_it->first.c_str(), local_it->second);
    }
  }
  */
  char const* cname = _cname;
  ControllerActionMapper* cam;
  try {
    cam = &(controller_mapper.at(cname));
  } catch (std::out_of_range oor) {
    try {
      cname = controller_name_mapper.at(std::string(cname));
      cam = &(controller_mapper.at(cname));
    } catch (std::out_of_range oor) {
      puts("Couldn't find the controller!");
      return;
    }
  }
  auto* f0 = static_cast<func<void(BaseController*,Request&,Response&)>*>(cam->get(mname));
  if (f0 == nullptr) {
    puts("null");
  } else {
    auto req = Request();
    auto res = Response();

    BaseController* bcp = controller_factory_mapper.at(cname)->create();
    if (bcp == nullptr) {
      puts("bcp null");
    } else {
      (*f0)(std::move(bcp), req, res);
      delete bcp;
    }
  }
}

//controller-extern.h
extern "C" void controller_handle_request(char const* path) {
  std::string p{path};
  p = p.substr(0, p.size()-1);

  std::vector<std::string> parts;
  split(parts, p, "/");
  if (parts.size() == 1) {
    dyncall(parts[0].c_str(), "index");
  } else if (parts.size() == 2) {
    printf(">>%s<<\n", parts[0].c_str());
    printf(">>%s<<\n", parts[1].c_str());
    dyncall(parts[0].c_str(), parts[1].c_str());
  } else {
    printf("<?> Can't handle /%s", path);
  }

}

