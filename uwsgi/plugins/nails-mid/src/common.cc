#include <stdint.h>

#include "common.h"
#include "uwsgi-includer.h"

std::string empty{};


void dump_wsgi_info(struct wsgi_request *wsgi_req) {
  debug_uwsgi_log("method(%hu): %s\n", wsgi_req->method_len, wsgi_req->method);
  debug_uwsgi_log("uri(%hu): %s\n", wsgi_req->uri_len, wsgi_req->uri);
  debug_uwsgi_log("scriptName(%hu): %s\n", wsgi_req->script_name_len, wsgi_req->script_name);
  debug_uwsgi_log("pathInfo(%hu): %s\n", wsgi_req->path_info_len, wsgi_req->path_info);
  debug_uwsgi_log("queryString(%hu): %s\n", wsgi_req->query_string_len, wsgi_req->query_string);
  debug_uwsgi_log("host(%hu): %s\n", wsgi_req->host_len, wsgi_req->host);
  debug_uwsgi_log("protocol(%hu): %s\n", wsgi_req->protocol_len, wsgi_req->protocol);
  debug_uwsgi_log("cookie(%hu): %s\n", wsgi_req->cookie_len, wsgi_req->cookie);
  if (wsgi_req->scheme_len) {
    //takes X-Forwarded-Proto when running in http mode
    //that seems dumb
    debug_uwsgi_log("scheme(%hu): %s\n", wsgi_req->scheme_len, wsgi_req->scheme);
  } else {
    debug_uwsgi_log("using http://");
  }
  debug_uwsgi_log("remoteAddr(%hu): %s\n", wsgi_req->remote_addr_len, wsgi_req->remote_addr);

  if (wsgi_req->remote_user_len) {
    debug_uwsgi_log("remoteUser(%hu): %s\n", wsgi_req->remote_user_len, wsgi_req->remote_user);
  } else {
    debug_uwsgi_log("no remote user");
  }
  debug_uwsgi_log("Content-Length: %lu\n", wsgi_req->post_cl);
  debug_uwsgi_log("Content-Type(%hu): %s\n", wsgi_req->content_type_len, wsgi_req->content_type);
  debug_uwsgi_log("wsgi_req->var_cnt: %hu\n", wsgi_req->var_cnt);
  debug_uwsgi_log("wsgi_req->header_cnt: %hu\n", wsgi_req->header_cnt);
  for (int i = 0; i < wsgi_req->var_cnt; i += 2) {
    debug_uwsgi_log("%.*s: %.*s\n", wsgi_req->hvec[i].iov_len, wsgi_req->hvec[i].iov_base, wsgi_req->hvec[i+1].iov_len, wsgi_req->hvec[i+1].iov_base);
  }
}


std::vector<std::string>
split(std::string const& original, char const separator) {
  size_t n = std::count(original.cbegin(), original.cend(), separator);
  std::vector<std::string> results;
  results.reserve(n+2);
  std::string::const_iterator start = original.cbegin();
  std::string::const_iterator end = original.cend();
  std::string::const_iterator next = std::find(start, end, separator);

  while (next != end) {
    results.push_back(std::string(start, next));
    start = next + 1;
    next = std::find(start, end, separator);
  }
  results.push_back(std::string(start, next));
  return results;
}

inline uint8_t unhexify(char c) {
  switch (c) {
    case '0':
      return 0x00;
    case '1':
      return 0x01;
    case '2':
      return 0x02;
    case '3':
      return 0x03;
    case '4':
      return 0x04;
    case '5':
      return 0x05;
    case '6':
      return 0x06;
    case '7':
      return 0x07;
    case '8':
      return 0x08;
    case '9':
      return 0x09;
    case 'a':
      return 0x0a;
    case 'b':
      return 0x0b;
    case 'c':
      return 0x0c;
    case 'd':
      return 0x0d;
    case 'e':
      return 0x0e;
    case 'f':
      return 0x0f;
    case 'A':
      return 0x0a;
    case 'B':
      return 0x0b;
    case 'C':
      return 0x0c;
    case 'D':
      return 0x0d;
    case 'E':
      return 0x0e;
    case 'F':
      return 0x0f;
    default:
      return 0xff;
  }
}

uint16_t decodeURLEscape(char h, char l) {
  uint16_t ret = 0;
  uint8_t v = unhexify(h);
  if (v == 0xff) {
    return static_cast<uint16_t>(0xffff);
  }
  ret = v << 4;

  v = unhexify(l);
  if (v == 0xff) {
    return static_cast<uint16_t>(0xffff);
  }
  ret |= v;
  return ret;
}

/*
std::string decodeURLString(std::string::const_iterator start,
                            std::string::const_iterator end) {

  std::string ret;
  ret.reserve(end-start);
  for (std::string::const_iterator c(start); c < end; c++) {
    if (*c == '%') {
      if (c+2 >= end) {
        return ret; //hahaha, i don't care
      } else {
        uint16_t dec = decodeURLEscape(*(c+1), *(c+2));
        if (dec == 0xffff) {
          //ret += '%';
          //suck it skids
          return ret;
        } else if (dec == 0x0000) {
          return ret; //imma not let you finish
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
*/

std::vector<std::string>
split_decode(std::string const& original, char const separator) {
  size_t n = std::count(original.cbegin(), original.cend(), separator);
  std::vector<std::string> results;
  results.reserve(n+2);
  std::string::const_iterator start = original.cbegin();
  std::string::const_iterator end = original.cend();
  std::string::const_iterator next = std::find(start, end, separator);

  while (next != end) {
    results.push_back(decodeURLString(start, next));
    start = next + 1;
    next = std::find(start, end, separator);
  }
  results.push_back(decodeURLString(start, next));
  return results;
}



std::string unslash(std::string::const_iterator start,
                    std::string::const_iterator end) {

  std::string ret;
  ret.reserve(end-start);
  for (std::string::const_iterator c(start); c < end; c++) {
    if (*c == '\\') {
      if (c+1 < end) {
        if (*(c+1) == '/') {
          continue;
        }
      }
    }
    ret += *c;
  }
  return ret;
}


// \/ in a route matches %2f in a path segment
std::vector<std::string>
split_escape(std::string const& original,
             char const separator, char const escape) {
  size_t n = 0;
  if (original[0] == separator) {
    n = 1;
  }
  for (auto c = original.cbegin()+1; c != original.cend(); ++c) {
    if (*c == separator && (&*c)[-1] != escape) {
      n += 1;
    }
  }

  std::vector<std::string> results;
  results.reserve(n+2);

  std::string::const_iterator start;
  std::string::const_iterator end = original.cend();
  std::string::const_iterator next;

  auto finder = [separator,escape](char const& c) {
    if (c == separator) {
      char const* cb = std::addressof(c);
      if (cb[-1] == escape) {
        return false;
      }
      return true;
    }
    return false;
  };

  if (original[0] == separator) {
    start = original.cbegin();
    next = std::find(start, end, separator);
  } else {
    start = original.cbegin()+1;
    next = std::find_if(start, end, finder);
  }
  
  while (next != end) {
    results.push_back(unslash(start, next));
    start = next + 1;
    next = std::find_if(start, end, finder);
  }
  results.push_back(unslash(start, next));
  return results;
}


bool startsWith(std::string const& str, std::string const& head) {
  size_t strL = str.size(), headL = head.size();
  if (strL >= headL) {
    return 0 == str.compare(0, headL, head);
  }
  return false;
}

bool endsWith(std::string const& str, std::string const& tail) {
  size_t strL = str.size(), tailL = tail.size();
  if (strL >= tailL) {
    return 0 == str.compare(strL - tailL, tailL, tail);
  }
  return false;
}
