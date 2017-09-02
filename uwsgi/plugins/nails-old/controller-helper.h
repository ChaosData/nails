#ifndef _CONTROLLER_HELPER_H_
#define _CONTROLLER_HELPER_H_

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <stdexcept>
#include <cctype>
#include <algorithm>

#include "common.h"
#include "controller-extern.h"
#include "nails_route_annotations.h"
#include "Request.h"

class Response { };

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

  ReturnType operator()(Params&&... params) {
      return f(std::forward<Params>(params)...);
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
  ControllerActionMapper() { }

  void add(char const* name, func_base* f) {
    exported_funcs.emplace(std::make_pair(name, f));
  }

  func_base* get(char const* name) {
    try {
      return exported_funcs.at(name);
    } catch (std::out_of_range oor) {
      return nullptr;
    }
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

typedef std::unordered_map<char const*,ControllerActionMapper> ControllerMapper;
extern ControllerMapper controller_mapper;


class factory_base {
 public:
  virtual ~factory_base(void) { }
  virtual BaseController* create() = 0;
};

template<
  typename T,
  typename = std::enable_if_t<std::is_base_of<BaseController, T>::value>
>
class ControllerFactory : public factory_base {

  BaseController* create() {
    return new T();
  }
};

typedef std::unordered_map<char const*,factory_base*> ControllerFactoryMapper;
extern ControllerFactoryMapper controller_factory_mapper;

template<typename T>
constexpr char const* type_name(T&& obj) {
  return typeid(std::forward<T>(obj)).name();
}

typedef std::unordered_map<std::string, char const*> ControllerNameMapper;
extern ControllerNameMapper controller_name_mapper;


void dyncall(char const* _cname, char const* mname);

struct split {
  enum empties_t { empties_ok, no_empties };
};

template <typename Container>
Container& split(Container& result,
                 const typename Container::value_type& s,
                 const typename Container::value_type& delimiters,
                 split::empties_t empties = split::empties_ok) {
  result.clear();
  size_t current;
  size_t next = -1;
  do {
    if (empties == split::no_empties) {
      next = s.find_first_not_of( delimiters, next + 1 );
      if (next == Container::value_type::npos) {
        break;
      }
      next -= 1;
    }
    current = next + 1;
    next = s.find_first_of( delimiters, current );
    result.push_back( s.substr( current, next - current ) );
  }while (next != Container::value_type::npos);
  return result;
}

extern std::string empty;

class Route {
 public:
  Route(std::string&& _path, std::set<std::string>&& _methods,
        func_base* _action, char const* _controller) :
    path_(std::move(_path)), methods_(std::move(_methods)),
    action_(_action), factory_(controller_factory_mapper.at(_controller)) {
    //pass
  }

  std::pair<factory_base*,func_base*>
  match(std::string const& _path, std::string const& _method) const {
    if ( path_ == _path && methods_.find(_method) != methods_.end() ) {
      return std::make_pair(factory_, action_);
    }
    return std::make_pair(nullptr,nullptr);
  }

 private:
  std::string path_;
  std::set<std::string> methods_;
  func_base* action_;
  factory_base* factory_;
};

class Router {
//router.add("/foo/:id", {"GET"}, make_func(&FooController::show), type_name(singleton_FooController));
 public:
  void add(Route&& _route) {
    routes.emplace_back(std::move(_route));
  }
  void add(std::string&& path, std::set<std::string>&& methods,
           func_base* action, char const* controller) {
    routes.emplace_back(std::move(path), std::move(methods),
                        action, controller);
  }

  bool route(std::string const& _path, std::string const& _method, Request& req, Response& res) {
    for (auto const& r : routes) {
      auto p = r.match(_path, _method);
      factory_base* fac = p.first;
      if (fac == nullptr) {
        continue;
      }
      func_base* fun = p.second;
      if (fun == nullptr) {
        continue;
      }
      BaseController* bcp = fac->create();
      if (bcp == nullptr) {
        //error
      } else {
        auto* mf = static_cast<func<void(BaseController*,Request&,Response&)>*>(fun);
        (*mf)(std::move(bcp), req, res);
        delete bcp;
        return true;
      }
    }
    return false;
  }
 private:
  std::vector<Route> routes; //not optimal
};

extern Router router;

#endif
