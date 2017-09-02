#include <stdio.h>
#include <string>
#include <unordered_map>

class func_base {
 public:
  virtual ~func_base() { };
};

template <typename ReturnType, typename... Params>
class func;

template <typename ReturnType, typename... Params>
class func<ReturnType(Params...)> : public func_base {
public:
  func(const std::function<ReturnType(Params...)>&& f) : f(std::move(f)) {}

  ReturnType operator()(Params... params) {
      return f(params...);
  }
  virtual ~func() { };

private:
  std::function<ReturnType(Params...)> f;
};


template <typename ReturnType, typename... Params>
func<ReturnType(Params...)>* make_func(ReturnType(*f)(Params...)) {
  return new func<ReturnType(Params...)>(std::function<ReturnType(Params...)>(f));
}

template <typename ReturnType, typename NS, typename... Params>
func<ReturnType(NS*,Params...)>* make_func(ReturnType(NS::*f)(Params...)) {
  return new func<ReturnType(NS*,Params...)>(std::function<ReturnType(NS*,Params...)>(f));
}


class BaseController {
 public:
  virtual ~BaseController(void) = default;

 private:
};

class ControllerActionMapper {
 public:
  void add(std::string const&& name, func_base* f) {
    exported_funcs.emplace(std::make_pair(std::move(name), f));
  }
  func_base* get(std::string& name) {
    return exported_funcs.at(name);
  }

  virtual ~ControllerActionMapper() {
    for (auto const& iter : exported_funcs) {
      delete iter.second;
    }
  }

 private:
  friend class BaseController;
  std::unordered_map<std::string,func_base*> exported_funcs;
};

typedef std::unordered_map<std::string,ControllerActionMapper> ControllerMapper;
static ControllerMapper controller_mapper;

class TestController : public BaseController {
 public:
  TestController(ControllerActionMapper& _mapper) : mapper_(_mapper) { }
  int foo(int a) { puts("gg"); return a; }
  static int foo2(int a) { puts("gg2"); return a; }

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
    auto* fb2 = mapper_.get(name);
    puts(typeid(*fb2).name());
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

//to be done via code gen
void init_controllers() {
  controller_mapper.emplace("TestController", ControllerActionMapper());
  auto& tcam = controller_mapper.at("TestController");
  tcam.add("foo", make_func(&TestController::foo));
  tcam.add("foo2", make_func(&TestController::foo2));
}


int main() {
  //to be called by library init
  init_controllers();


  TestController a(controller_mapper.at("TestController"));
  a.bar("foo");
  a.bar2("foo2");
  return 0;
}
