#ifndef FORMAT_STRING_H_
#define FORMAT_STRING_H_

#include <string>
#include <limits>
#include <vector>

struct format_string {
 public:
  template<typename... T>
  format_string (T&&... params) : fmt(std::forward<T>(params)...) { }

  template<typename... P>
  std::string operator()(P&&... args) {
    int size = snprintf(nullptr, 0, fmt.c_str(), (args)...);
    if (size == std::numeric_limits<int>::max()) {
      return std::string();
    }
    size += 1;
    std::vector<char> data;
    data.assign(size, '\0'); //std::string::data is a dull boy
    snprintf(data.data(), size, fmt.c_str(), (args)...);
    std::string out(data.data());
    return out;
  }
 private:
  std::string fmt;
};

format_string operator"" _format (char const* str, unsigned long sz) {
  return format_string(str, sz);
}

#endif
