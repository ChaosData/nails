#ifndef _NAILS_ROUTE_ANNOTATIONS_H_
#define _NAILS_ROUTE_ANNOTATIONS_H_

#include <type_traits>
#include <string>
#include <stdio.h>
#include <array>
#include <algorithm>
#include "str_const.h"

namespace nails {

class Response;

template<bool... bs>
struct multi_bool {
  static constexpr bool multi_and(std::array<bool, sizeof...(bs)> const& arr) {
    for (size_t i(0); i < sizeof...(bs); i++) {
      if (arr[i] == false) {
        return false;
      }
    }
    return true;
  }

  static constexpr std::array<bool, sizeof...(bs)> values = { { bs... } };
  static constexpr bool value = multi_and(values);
};


template<typename Key, typename... Ts>
struct all_are {
  all_are(Key&, Ts&...) { }

  static constexpr bool all_are_same() {
    return multi_bool<(std::is_same<Key, Ts>::value)...>::value;
  }
  static constexpr bool value = all_are_same();
};

struct nails_url {
  constexpr nails_url() { }

  constexpr bool operator() (str_const const&) const {
    return true;
  }

/*  constexpr bool operator= (str_const const&) const {
    return true;
  }*/
};
using Url = nails_url const&;

struct nails_methods {
  constexpr nails_methods() { }

  template<
    typename... Ts,
    std::enable_if_t<multi_bool<(std::is_convertible<Ts, str_const>::value)...>::value>* = nullptr
  >
  constexpr bool operator() (Ts&&...) const {
    return true;
  }

};
using Methods = nails_methods const&;

template<bool... b>
using app_route = std::enable_if_t<multi_bool<b...>::value,Response>;



extern nails_url const url;
extern nails_methods const methods;
};

using nails::app_route;
using nails::Url;
using nails::Methods;

#define nails_route template<Url url, Methods methods> app_route

using nails::url;
using nails::methods;
#define nails_route_explicit(clazz) template Response clazz<url,methods>(Request& req)


#endif