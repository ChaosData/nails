#ifndef _NAILS_SIGNVERIFY_H_
#define _NAILS_SIGNVERIFY_H_

#include "sodium.h"
#include "stdint.h"

namespace nails {

constexpr size_t mac_hex_len = crypto_auth_BYTES*2 + 1;
constexpr size_t key_hex_len = crypto_auth_KEYBYTES*2 + 1;

char* sign(char const* msg, uint64_t msg_len, uint8_t const* key);

bool verify(char const* msg, uint64_t msg_len, char const* mac_hex,
            uint8_t const* key);

bool generate_key_and_hex(uint8_t* _key, char* _key_hex);

uint8_t* load_key(char const* path);


}; // namespace nails

#endif