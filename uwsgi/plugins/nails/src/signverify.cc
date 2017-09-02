#include "sodium.h" // vendor/libsodium/src/libsodium/.libs/libsodium.a
                    // vendor/libsodium/src/libsodium/include/sodium.h
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <fstream>
#include <string>
#include <memory>

#include "signverify.h"

namespace nails {

char* sign(char const* msg, uint64_t msg_len, uint8_t const* key) {
  unsigned char mac[crypto_auth_BYTES];

  crypto_auth(mac, reinterpret_cast<unsigned char const*>(msg), msg_len, key);

  char* mac_hex = (char*)malloc(mac_hex_len);

  char* out = sodium_bin2hex(mac_hex, mac_hex_len, mac, sizeof(mac));
  if (out == nullptr) {
    free((void*)mac_hex);
    return nullptr;
  }
  return mac_hex;
}

bool verify(char const* msg, uint64_t msg_len, char const* mac_hex,
            uint8_t const* key) {
  unsigned char mac[crypto_auth_BYTES];
  size_t mac_len = sizeof(mac);
  if(sodium_hex2bin(mac, sizeof(mac), mac_hex, mac_hex_len, nullptr,
                    &mac_len, nullptr) != 0) {
    return false;
  }

  return crypto_auth_verify(mac, (uint8_t const*)msg, msg_len, key) == 0;
}


bool generate_key_and_hex(uint8_t* _key, char* _key_hex) {
  randombytes_buf(_key, crypto_auth_KEYBYTES);
  char* out = sodium_bin2hex(_key_hex, key_hex_len, _key, crypto_auth_KEYBYTES);
  if (out != nullptr) {
    return true;
  } else {
    return false;
  }
}

uint8_t* load_key(char const* path) {
  uint8_t* key = (uint8_t*)sodium_malloc(crypto_auth_KEYBYTES);
  size_t key_len = crypto_auth_KEYBYTES;
  char* key_hex = (char*)sodium_malloc(key_hex_len);

  std::string chmodur = "chmod u+r ";
  chmodur.append(path);
  std::string chmodnor = "chmod -r ";
  chmodnor.append(path);

  (void)system(chmodur.c_str());

  bool fail = true;
  std::ifstream keyfile{path};
  keyfile.read((char*)key_hex, key_hex_len);
  size_t read = keyfile.gcount();
  if (read == 0) {
    //need to create key
    puts("creating key");
    //randombytes_buf(key, crypto_auth_KEYBYTES);
    //char* out = sodium_bin2hex(key_hex, key_hex_len, key, crypto_auth_KEYBYTES);

    //if (out == nullptr) {
    if (!generate_key_and_hex(key, key_hex)) {
      puts("key generation error");
    } else {
      std::ofstream keyfileout{path};
      keyfileout.write((char const*)key_hex, key_hex_len);
      keyfileout.flush();
      keyfileout.close();
      fail = false;
    }
  } else if (read == key_hex_len) {
    //got key
    if(sodium_hex2bin((uint8_t*)key, sizeof(key), (char const*)key_hex,
                      key_hex_len, nullptr, &key_len, nullptr) != 0) {
      puts("invalid key file");
    } else {
      fail = false;
    }
  } else {
    puts("incorrect key file read size");
  }

  if (fail) {
    sodium_free(key);
    key = nullptr;
  }

  sodium_free(key_hex);
  (void)system(chmodnor.c_str());

  return key;
}


/*
int main(int argc, char* argv[]) {
  if (sodium_init() == -1) {
    puts("can't sodium_init()!");
    return 1;
  }


  if (isX(name, "sign")) {
    if (argc != 2) {
      puts("usage: ./sign '<data>'");
      return 5;
    }
    auto mac = std::unique_ptr<char const>{sign(argv[1], strlen(argv[1]), key)};
    if (mac == nullptr) {
      puts("signing failed");
      return 6;
    }
    puts(mac.get());
  } else if (isX(name, "verify")) {
    if (argc != 3) {
      puts("usage: ./verify '<mac>' '<data>'");
      return 7;
    }

    if (verify(argv[2], strlen(argv[2]), argv[1], key)) {
      puts("Signature OK!");
    } else {
      puts("Signature BAD!");
    }
  } else {
    puts("wrong name");
    return 3;
  }
  return 0;
}
*/

}; // namespace nails
