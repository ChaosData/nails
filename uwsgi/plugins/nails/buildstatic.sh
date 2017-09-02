#!/bin/sh

if [ "$#" -ne 1 ]; then
  ~/code/c++/clang-llvm/build/bin/nails-preroute app/controllers/*.cc app/controllers/*.hpp -- -std=c++14 -stdlib=libc++ -Iincludes
fi

for file in $(ls src/ | grep '\.cc'); do
  clang++ -std=c++14 -stdlib=libc++ \
    -Wall -Wextra -Wno-unused-variable -pedantic -Wno-c99-extensions -g \
    -c -o build/$file.o \
    -Iincludes \
    src/$file
done;

for file in $(ls gen/ | grep '\.cc'); do
  clang++ -std=c++14 -stdlib=libc++ \
    -Wall -Wextra -Wno-unused-variable -pedantic -Wno-c99-extensions -g \
    -c -o build/$file.o \
    -Iincludes \
    gen/$file
done;

for file in $(ls app/controllers/ | grep '\.cc'); do
  clang++ -std=c++14 -stdlib=libc++ \
    -Wall -Wextra -Wno-unused-variable -pedantic -g \
    -c -o build/$file.o \
    -Iincludes \
    app/controllers/$file
done;

ar rcs output/libnails.a build/*.o

