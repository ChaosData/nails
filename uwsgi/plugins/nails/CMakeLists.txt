cmake_minimum_required (VERSION 3.4)
project (Nails CXX)

include_directories(
  "${PROJECT_SOURCE_DIR}/include"
  "${PROJECT_SOURCE_DIR}/gen"
  "${PROJECT_SOURCE_DIR}/config"
  "${PROJECT_SOURCE_DIR}/vendor/base64/include"
  "${PROJECT_SOURCE_DIR}/vendor/libsodium/src/libsodium/include"
)


file(GLOB_RECURSE CONTROLLERS
  RELATIVE ${PROJECT_SOURCE_DIR}

  app/controllers/*.cc
  app/controllers/*.cpp
)

file(GLOB_RECURSE CONTROLLERS_HPP
  RELATIVE ${PROJECT_SOURCE_DIR}

  ${PROJECT_SOURCE_DIR}/app/controllers/*.hpp
)

file(GLOB_RECURSE INITIALIZERS_HPP
  RELATIVE ${PROJECT_SOURCE_DIR}

  ${PROJECT_SOURCE_DIR}/config/initializers/*.hpp
)


add_custom_command(
  OUTPUT gen/gen.cc gen/gen.h
  COMMAND vendor/clang-llvm/build/bin/nails-preroute ${CONTROLLERS} ${CONTROLLERS_HPP} -- -std=c++14 -stdlib=libc++ -Iinclude
  DEPENDS ${CONTROLLERS} ${CONTROLLERS_HPP} vendor/clang-llvm/build/bin/nails-preroute
  COMMENT "Generating routes..."
)

add_custom_target(RouteGenerator
  DEPENDS gen/gen.cc gen/gen.h 
  COMMENT "Checking if route re-generation is required"
)

add_library(Nails STATIC
  src/common.cc
  src/http.cc
  src/base.cc
  src/controller-helper.cc
  src/reqres.cc
  src/signverify.cc
  src/filters.cc
  src/router.cc
  src/nails_route_annotations.cc
  src/template.cc
  gen/gen.cc
  ${CONTROLLERS}
  config/initializers.cc
  vendor/base64/src/base64.cc
)

add_dependencies(Nails RouteGenerator)

target_compile_features(Nails PUBLIC cxx_generic_lambdas)

set_target_properties(Nails
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-unused-variable -Wno-c99-extensions -fPIC -fPIE -fstack-protector-strong -D_FORTIFY_SOURCE=2 -O2")

add_library(libsodium STATIC IMPORTED)
set_target_properties(libsodium PROPERTIES
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/vendor/libsodium/src/libsodium/.libs/libsodium.a"
)
target_link_libraries(Nails libsodium)

# for uwsgi
# env CFLAGS="-fPIC -fPIE -fstack-protector-all -D_FORTIFY_SOURCE=2" LDFLAGS="-fPIC -fPIE -pie -Wl,-z,relro,-z,now,-z,noexecstack" make
