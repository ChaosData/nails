#ifndef _CONTROLLER_HELPER_H_
#define _CONTROLLER_HELPER_H_

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <algorithm>

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

  //BaseController* cw_;

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

#endif
