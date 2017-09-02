#ifndef STR_CONST_H_
#define STR_CONST_H_

#include <stdexcept>

class str_const { // constexpr string
 private:
  const char* const p_;
  const std::size_t sz_;

 public:
/*  template<std::size_t N>
  constexpr str_const() : // ctor
      p_(nullptr), sz_(0) {
  }

  constexpr str_const(str_const&) : p_(nullptr), sz_(0) {
  }

  constexpr str_const(str_const&&) : p_(nullptr), sz_(0) {
  }
*/
  template<std::size_t N>
  constexpr str_const(const char(&a)[N]) : // ctor
      p_(a), sz_(N-1) {
  }

  constexpr char operator[](std::size_t n) const { // []
    return n < sz_ ?
        p_[n] :
        throw std::out_of_range("");
  }

  constexpr std::size_t size() const { return sz_; } // size()

  constexpr bool operator ==(str_const const& rhs) const {
    if (sz_ != rhs.size()) {
      return false;
    }
    for (size_t i(0); i < sz_; i++) {
      if (p_[i] != rhs[i]) {
        return false;
      }
    }
    return true;
  }

  template<std::size_t N>
  constexpr bool operator ==(const char(&rhs)[N]) const {
    if (sz_ != N - 1) {
      return false;
    }
    for (size_t i(0); i < sz_; i++) {
      if (p_[i] != rhs[i]) {
        return false;
      }
    }
    return true;
  }

  template<std::size_t N>
  constexpr bool operator ()(const char(&rhs)[N]) const {
    return this == rhs;
  }
};

#endif
