#include <stdio.h>
#include <string>
#include <utility>
#include <unordered_map>
#include <typeinfo>

class Params { };
class Request { };
class Response { };

class router { };

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


class controller_base {
 public:
  friend class router;
  controller_base() { }
  virtual int request() { return 404; }

  virtual ~controller_base() {
    for (auto const& iter : exported_funcs) {
      delete iter.second;
    }
  }
 protected:
  void register_func(std::string const&& name, func_base* f) {
    exported_funcs.emplace(std::make_pair(std::move(name), f));
  }

 protected:
  std::unordered_map<std::string,func_base*> exported_funcs;
};

class TestController : public controller_base {
 public:
  int request() { return 200; }
  int foo(int a) { puts("gg"); return a; }
  static int foo2(int a) { puts("gg2"); return a; }


  void bar(std::string&& name) {
    auto* fb = dynamic_cast<func<int(TestController*,int)>*>(exported_funcs.at(name));
    if (fb == nullptr) {
      puts("null");
    } else {
      (*fb)(this, 5);
    }
  }
  void bar2(std::string&& name) {
    auto* fb = dynamic_cast<func<int(int)>*>(exported_funcs.at(name));
    if (fb == nullptr) {
      puts("null");
    } else {
      (*fb)(5);
    }
  }


  TestController() {
    register_func("foo", make_func(&TestController::foo));
    register_func("foo2", make_func(&TestController::foo2));
  }
};

int main() {
  TestController a;
  a.bar("foo");
  a.bar2("foo2");
  return 0;
}
