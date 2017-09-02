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

bool const valid_uri_byte[256]{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};


char const* const url_encodes[256]{
  "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
  "%08", "%09", "%0a", "%0b", "%0c", "%0d", "%0e", "%0f", 
  "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
  "%18", "%19", "%1a", "%1b", "%1c", "%1d", "%1e", "%1f", 
  "%20", "!",   "%22", "%23", "%24", "%25", "%26", "'",
  "(",   ")",   "*",   "%2b", "%2c", "-",   ".",   "%2f", 
  "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
  "8",   "9",   "%3a", "%3b", "%3c", "%3d", "%3e", "%3f", 
  "%40", "A",   "B",   "C",   "D",   "E",   "F",   "G",
  "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O", 
  "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
  "X",   "Y",   "Z",   "%5b", "%5c", "%5d", "%5e", "_", 
  "%60", "a",   "b",   "c",   "d",   "e",   "f",   "g",
  "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o", 
  "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
  "x",   "y",   "z",   "%7b", "%7c", "%7d", "~",   "%7f", 
  "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
  "%88", "%89", "%8a", "%8b", "%8c", "%8d", "%8e", "%8f", 
  "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
  "%98", "%99", "%9a", "%9b", "%9c", "%9d", "%9e", "%9f", 
  "%a0", "%a1", "%a2", "%a3", "%a4", "%a5", "%a6", "%a7",
  "%a8", "%a9", "%aa", "%ab", "%ac", "%ad", "%ae", "%af", 
  "%b0", "%b1", "%b2", "%b3", "%b4", "%b5", "%b6", "%b7",
  "%b8", "%b9", "%ba", "%bb", "%bc", "%bd", "%be", "%bf", 
  "%c0", "%c1", "%c2", "%c3", "%c4", "%c5", "%c6", "%c7",
  "%c8", "%c9", "%ca", "%cb", "%cc", "%cd", "%ce", "%cf", 
  "%d0", "%d1", "%d2", "%d3", "%d4", "%d5", "%d6", "%d7",
  "%d8", "%d9", "%da", "%db", "%dc", "%dd", "%de", "%df", 
  "%e0", "%e1", "%e2", "%e3", "%e4", "%e5", "%e6", "%e7",
  "%e8", "%e9", "%ea", "%eb", "%ec", "%ed", "%ee", "%ef", 
  "%f0", "%f1", "%f2", "%f3", "%f4", "%f5", "%f6", "%f7",
  "%f8", "%f9", "%fa", "%fb", "%fc", "%fd", "%fe", "%ff",
};

inline char const* urlify(char c) {
  return url_encodes[(uint8_t)c];
}

std::string encodeURIComponent(std::string const& in) {
  //encodeURIComponent escapes all characters except the following:
  //  alphabetic, decimal digits, - _ . ! ~ * ' ( )

  std::string out;
  out.reserve(in.size()*2); //hedging on 2x against 3x

  for (auto c : in) {
    out += urlify(c);
  }
  return out;
}

/*
std::string decodeURIComponent(std::string::const_iterator start,
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
    results.push_back(decodeURIComponent(start, next));
    start = next + 1;
    next = std::find(start, end, separator);
  }
  results.push_back(decodeURIComponent(start, next));
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
