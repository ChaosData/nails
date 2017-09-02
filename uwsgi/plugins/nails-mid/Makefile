CXX=clang++
CXXFLAGS=-std=c++14 -stdlib=libc++ -Wall -Wextra -pedantic -fPIC -fPIE -fstack-protector-strong -D_FORTIFY_SOURCE=2
SANITIZE=-fsanitize=address,undefined
DEBUG=-g -DDEBUG
OPTIMIZE=-O2
INCS=-I include
LINK=-Wl,-z,relro,-z,now,-z,noexecstack
OUTPUT=-shared -o lib/libnails.so


default: build/*.o
	${CXX} ${CXXFLAGS} ${OPTIMIZE} ${LINK} ${OUTPUT} ${INCS} build/*.o
	ar rcs lib/libnails.a build/*.o

debug: build/*.o
	${CXX} ${CXXFLAGS} ${DEBUG} ${SANITIZE} ${LINK} ${OUTPUT} ${INCS} build/shambles.o build/shambles_intercept.o build/forgery.o  build/util.o
	ar rcs lib/libnails.a build/*.o

vdebug: build/*.o
	${CXX} ${CXXFLAGS} ${DEBUG} ${LINK} ${OUTPUT} ${INCS} build/shambles.o build/shambles_intercept.o build/forgery.o  build/util.o
	ar rcs lib/libnails.a build/*.o

build/__.o: src/__.cc
	${CXX} ${CXXFLAGS} ${INCS} -o build/__.o -c src/__.cc

clean:
	rm lib/libnails.so lib/libnails.a build/*.o
