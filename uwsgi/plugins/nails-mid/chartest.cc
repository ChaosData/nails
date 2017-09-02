#include <stdint.h>
#include <stdio.h>
#include <string>

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
$ ./chartest | hd
00000000  41 ff 0a                                          |A..|
00000003
*/
int main() {
  std::string gg;
  gg += (char)decodeURLEscape('4', '1');
  gg += (char)decodeURLEscape('f', 'F');

  puts(gg.c_str());
  return 0;
}
