#ifndef COMMON_H_
#define COMMON_H_

#include <string>
#include <vector>

#ifdef NAILS_DEBUG
  #define debug_printf(...) printf(__VA_ARGS__)
  #define debug_puts(...) puts(__VA_ARGS__)
  #define debug_uwsgi_log(...) uwsgi_log(__VA_ARGS__)
#else
  #define debug_printf(...) (void)0
  #define debug_puts(...) (void)0
  #define debug_uwsgi_log(...) (void)0
#endif



extern std::string empty;

struct wsgi_request;
void dump_wsgi_info(struct wsgi_request *wsgi_req);

std::vector<std::string>
split(std::string const& original, char const separator);

std::vector<std::string>
split_decode(std::string const& original, char const separator);

std::vector<std::string>
split_escape(std::string const& original,
             char const separator, char const escape = '\\');

uint16_t decodeURLEscape(char h, char l);

std::string encodeURIComponent(std::string const& in);


extern char const* const url_encodes[];
extern bool const valid_uri_byte[];

inline bool notValidURIByte(char c) {
  return !valid_uri_byte[(uint8_t)c];
}

template<typename T>
std::string decodeURIComponent(T start, T end) {
  std::string ret{};
  ret.reserve(end-start);
  for (T c(start); c < end; ++c) {
    // if (notValidURIByte(*c)) {
    //   return ret;
    // }
    if (*c == '%') {
      if (c+2 >= end) {
        return ret; //hahaha, i don't care
      } else {
        uint16_t dec = decodeURLEscape(*(c+1), *(c+2));
        if (dec == 0xffff) {
          //suck it skids
          return ret;
        } else if (dec == 0x0000) {
          return ret; //nope.jpg
        } else {
          ret += static_cast<char>(dec);
          c += 2;
        }
      }
    } else {
      ret += *c;
    }
  }
  return ret;
}

template<typename T>
std::string decodeURIComponent(T const& s) {
  return decodeURIComponent(s.cbegin(), s.cend());
}


template<typename T>
std::string decodeURIComponentPlus(T start, T end) {
  std::string ret{};
  ret.reserve(end-start);
  for (T c(start); c < end; ++c) {
    // if (notValidURIByte(*c) && *c != '+') {
    //   return ret;
    // }
    if (*c == '%') {
      if (c+2 >= end) {
        return ret; //hahaha, i don't care
      } else {
        uint16_t dec = decodeURLEscape(*(c+1), *(c+2));
        if (dec == 0xffff) {
          //suck it skids
          return ret;
        } else if (dec == 0x0000) {
          return ret; //nope.jpg
        } else {
          ret += static_cast<char>(dec);
          c += 2;
        }
      }
    } else if (*c == '+') {
      ret += " ";
    } else {
      ret += *c;
    }
  }
  return ret;
}

template<typename T>
std::string decodeURIComponentPlus(T const& s) {
  return decodeURIComponentPlus(s.cbegin(), s.cend());
}


bool startsWith(std::string const& str, std::string const& head);
bool endsWith(std::string const& str, std::string const& tail);

template<typename T, typename S>
bool mapHas(T const& m, S const& s) {
  if (m.find(s) != m.end()) {
    return true;
  } else {
    return false;
  }
}

#endif
