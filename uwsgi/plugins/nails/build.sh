#!/bin/sh

if [ "$#" -ne 1 ]; then
  ~/code/c++/clang-llvm/build/bin/nails-preroute controllers/*.cc controllers/*.hpp -- -std=c++14 -stdlib=libc++ -Iincludes
fi

clang++ -std=c++14 -stdlib=libc++ \
  -Wall -Wextra -Wno-unused-variable -pedantic -g \
  -shared -fPIC -o libs/libcontroller.so \
  -Iincludes \
  src/*.cc gen/*.cc controllers/*.cc
