#!/bin/sh

clang++ -std=c++14 -stdlib=libc++ \
  -Wall -Wextra -Wno-unused-variable -pedantic -g \
  -o main \
  -Iincludes -Igen \
  src/*.cc gen/*.cc controllers/*.cc \
  $1
