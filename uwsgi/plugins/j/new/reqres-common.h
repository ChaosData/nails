#ifndef _REQRES_COMMON_H_
#define _REQRES_COMMON_H_

#include <unordered_map>
#include <stdexcept>

#include "reqres-common.h"

template<typename A, typename B>
class dual_map {
 public:
  explicit dual_map(void) = delete;
  dual_map(std::unordered_map<A,B>& _one,
           std::unordered_map<A,B>& _two)
    : one(_one), two(_two) { }

  A* operator[] (A const& _a) noexcept {
    try {
      return &(one.at(_a));
    } catch (std::out_of_range oor) { }
    try {
      return &(two.at(_a));
    } catch (std::out_of_range oor) { return nullptr; }
  }

 private:
  std::unordered_map<A,B>& one;
  std::unordered_map<A,B>& two;
};

#endif
