#include "stdio.h"

#include "nails.h"

#include "base64.h"
#include "signverify.h"

#include "filters.h"
#include "common.h"

#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>

#include "sodium.h"

#include <string>
#include <memory>

//note: currently just implementing sessions using signed cookie_store.
//      i don't believe in encrypting this data. if you need to, you've already
//      done something wrong.


//note: from application code, if you want to flush the session, do:
//      req.session["expires"] = 0;


static uint8_t const* key = nullptr;
static std::string const cookie_name = "nails_session";
constexpr uint64_t const max_age = 6*60*60;

bool sessions_postmatch(Request& req) {
  auto session_cookie = req.cookies.find(cookie_name);
  if (session_cookie != req.cookies.end()) {
    std::string const& session_data = session_cookie->second;
    if (session_data.size() < 64) { //min length; e.g. just mac
      return false;
    }
    char const* msgb64 = session_data.c_str();
    size_t msgb64len = session_data.size()-64;
    char const* mac_hex = msgb64+msgb64len;

    //verify
    if(!nails::verify(msgb64, msgb64len, mac_hex, key)) {
      return false;
    }

    std::string msg;
    size_t msg_alloclen = apple_aprutil::Base64decode_len(msgb64, msgb64len);
    msg.resize(msg_alloclen);

    //rcs, eat your heart out
    char* msg_internal = &msg[0];

    size_t msglen = apple_aprutil::Base64decode(msg_internal, msgb64, msgb64len);
    msg.resize(msglen);

    try {
      req.session = json::parse(msg);
      auto expires_iter = req.session.find("expires");
      uint64_t expires = 1;
      if (expires_iter != req.session.end()) {
        expires = req.session["expires"].get<uint64_t>();
      }
      uint64_t now = static_cast<uint64_t>(time(nullptr));
      if (now > expires) {
        req.session.empty();
      }
    } catch (...) { }
  }
  return false;
}


bool sessions_response(Request& req, Response& res) {
  if (req.session.empty()) {
    return false;
  }

  if (mapHas(req.session, "expires")) {
    auto& expires_json = req.session["expires"];
    if (expires_json.get<uint64_t>() != 0) {
      expires_json = static_cast<uint64_t>(time(nullptr))+max_age;
    }
  } else {
    req.session["expires"] = static_cast<uint64_t>(time(nullptr))+max_age;
  }

  std::string json_dump = req.session.dump();
  size_t value_alloclen = apple_aprutil::Base64encode_len(json_dump.size());
  std::string value;
  value.resize(value_alloclen);

  size_t value_len = apple_aprutil::Base64encode(&value[0],
                                                    json_dump.c_str(),
                                                    json_dump.size());
  value_len -= 1; // NUL
  value.resize(value_len);

  auto mac = std::unique_ptr<char const>{
      nails::sign(value.data(), value_len, key)
  };
  value.append(mac.get(), nails::mac_hex_len-1); // NUL

  res.set_cookie(cookie_name, std::move(value), max_age);
  return false;
}

bool sessions_init() {
  puts("initializing sessions");

  #ifndef NAILS_DEBUG
  struct rlimit r{0,0};
  setrlimit(RLIMIT_CORE, &r);
  #endif

  if (sodium_init() == -1) {
    puts("can't sodium_init()!");
    return false;
  }

  key = nails::load_key("./nails_cookie_store_key");

  nails::postmatch_filters.emplace_back(
    std::make_pair(
      make_func(&sessions_postmatch),
      nullptr
    )
  );

  nails::response_filters.emplace_back(
    make_func(&sessions_response)
  );

  return true;
}

