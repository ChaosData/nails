#include "sodium.h" // vendor/libsodium/src/libsodium/.libs/libsodium.a
                    // vendor/libsodium/src/libsodium/include/sodium.h
#include "stdio.h"
#include "stdint.h"
#include "string.h"

#include <fstream>
#include <string>
#include <memory>

constexpr size_t mac_hex_len = crypto_auth_BYTES*2 + 1;
constexpr size_t key_hex_len = crypto_auth_KEYBYTES*2 + 1;

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

bool isX(std::string name, char const* key) {
  size_t pos = name.rfind(key);
  if (pos == std::string::npos) {
    return false;
  }
  if (pos+strlen(key) == name.size()) {
    return true;
  }
  return false;
}

int main(int argc, char* argv[]) {
  if (sodium_init() == -1) {
    puts("can't sodium_init()!");
    return 1;
  }

  unsigned char key[crypto_auth_KEYBYTES];
  size_t key_len = sizeof(key);
  char key_hex[key_hex_len];

  std::ifstream keyfile{"./key"};
  /*if (!keyfile.is_open()) {
    puts("can't open key file");
    return 2;
  }*/
  keyfile.read((char*)key_hex, key_hex_len);
  size_t read = keyfile.gcount();
  if (read == 0) {
    //need to create key
    puts("creating key");
    randombytes_buf(key, sizeof key);
    char* out = sodium_bin2hex((char*)key_hex, key_hex_len, key, sizeof(key));
    if (out == nullptr) {
      puts("key generation error");
      return 2;
    }
    std::ofstream keyfileout{"./key"};
    keyfileout.write((char const*)key_hex, key_hex_len);
    keyfileout.flush();
    keyfileout.close();
  } else if (read == key_hex_len) {
    //got key
    if(sodium_hex2bin((uint8_t*)key, sizeof(key), (char const*)key_hex, key_hex_len, nullptr,
                      &key_len, nullptr) != 0) {
      puts("invalid key file");
      return 3;
    }
  } else {
    puts("invalid key file");
    return 4;
  }

  std::string name{argv[0]};

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
