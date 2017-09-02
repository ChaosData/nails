#include <stdio.h>
#include <string>
#include <unordered_map>

#include "controller-helper.h"
#include "gen.h"

class TestController : public BaseController, public func_base {
 public:
  TestController(ControllerActionMapper& _mapper) : mapper_(_mapper) { }
  int foo(int a) { puts("gg"); return a; }
  static int foo2(int a) { puts("gg2"); return a; }

  void show(Request& req, Response& res) {
    (void)req;
    (void)res;
    puts("show()");
  }

  void show2(Request req, Request* req2, Request** req3, Response& res, Response&& res2) {
    (void)req;
    (void)res;
    (void)req2;
    (void)res2;
    (void)req3;
    puts("show()");
  }

  void index(Request& req, Response& res) {
    (void)req;
    (void)res;
    puts("index()");
    puts("index()");
  }


  //TODO:
  // - action handlers of the form:
  //     void show(Params params) { ... }
  //   - where the XxxController contains:
  //     - a bool flag for calling a view based on name (that can be set by manual render calls)
  //   - where Params is some form of hash/unordered_map
  // - before/after/etc filters


  //to be called from some sort of lower level request routing handler
  void bar(std::string&& name) {
    auto* fb = dynamic_cast<func<int(TestController*,int)>*>(mapper_.get(name));
    if (fb == nullptr) {
      puts("null");
    } else {
      (*fb)(this, 5);
    }
  }

  //to be called from some sort of lower level request routing handler
  void bar2(std::string&& name) {
    auto* fb = dynamic_cast<func<int(int)>*>(mapper_.get(name));
    if (fb == nullptr) {
      puts("null");
    } else {
      (*fb)(5);
    }
  }

  private:
    ControllerActionMapper& mapper_;
};



class Test2Controller : public BaseController, public func_base {
 public:
  Test2Controller(ControllerActionMapper& _mapper) : mapper_(_mapper) { }

  void show(Request& req, Response& res) {
    (void)req;
    (void)res;
    puts("2:show()");
  }

  void destroy(Request& req, Response& res) {
    (void)req;
    (void)res;
    puts("2:destroy()");
  }

 private:
  ControllerActionMapper& mapper_;
};

//to be done via code gen
void init_controllers() {
  controller_mapper.emplace("TestController", ControllerActionMapper());
  auto& tcam = controller_mapper.at("TestController");
  tcam.add("foo", make_func(&TestController::foo));
  tcam.add("foo2", make_func(&TestController::foo2));
}


void dyncall(std::string&& cname, std::string&& mname) {
  ControllerActionMapper& cam = controller_mapper.at(cname);
  auto* f = dynamic_cast<func<void(Request&,Response&)>*>(cam.get(mname));
  if (f == nullptr) {
    puts("null");
  } else {
    auto req = Request();
    auto res = Response();
    (*f)(req, res);
  }
}

int main() {
  //to be called by library init
  init_controllers();


  //TestController a(controller_mapper.at("TestController"));
  //a.bar("foo");
  //a.bar2("foo2");
  dyncall("TestController", "index");
  dyncall("Test2Controller", "destroy");
  return 0;
}
