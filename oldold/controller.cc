#include <stdio.h>
#include <string>
#include <unordered_map>

#include "controller-helper.h"
#include "controller.h"
#include "gen.h"

//TestController::TestController(ControllerActionMapper& _mapper) : mapper_(_mapper) { }
TestController::TestController() { }

void TestController::show(Request& req, Response& res) {
  (void)req;
  (void)res;
  puts("show()");
}

void TestController::show2(Request req, Request* req2, Request** req3, Response& res, Response&& res2) {
  (void)req;
  (void)res;
  (void)req2;
  (void)res2;
  (void)req3;
  puts("show()");
}

void TestController::index(Request& req, Response& res) {
  (void)req;
  (void)res;
  puts("index()");
  puts("index()");
}


//Test2Controller::Test2Controller(ControllerActionMapper& _mapper) : mapper_(_mapper) { }
Test2Controller::Test2Controller() { }

void Test2Controller::show(Request& req, Response& res) {
  (void)req;
  (void)res;
  puts("2:show()");
}

void Test2Controller::destroy(Request& req, Response& res) {
  (void)req;
  (void)res;
  puts("2:destroy()");
}

//to be done via code gen
/*void init_controllers() {
  controller_mapper.emplace("TestController", ControllerActionMapper());
  auto& tcam = controller_mapper.at("TestController");
  tcam.add("foo", make_func(&TestController::foo));
  tcam.add("foo2", make_func(&TestController::foo2));
}*/


void dyncall(char const* cname, char const* mname) {
  ControllerActionMapper& cam = controller_mapper.at(cname);
  auto* f0 = static_cast<func<void(BaseController*,Request&,Response&)>*>(cam.get(mname));
  if (f0 == nullptr) {
    puts("null");
  } else {
    auto req = Request();
    auto res = Response();
    //printf("dyncall: %s (%p)\n", typeid(*f0).name(), typeid(*f0).name());
    BaseController* bcp = controller_factory_mapper.at(cname)->create();
    if (bcp == nullptr) {
      puts("bcp null");
    } else {
      (*f0)(std::move(bcp), req, res);
      delete bcp;
    }
  }
}

int main() {
  //to be called by library init
  init_controllers();


  //TestController a(controller_mapper.at("TestController"));
  //a.bar("foo");
  //a.bar2("foo2");

  dyncall(type_name(singleton_TestController), "index");
  dyncall(type_name(singleton_Test2Controller), "destroy");


  teardown_controllers();
  return 0;
}
