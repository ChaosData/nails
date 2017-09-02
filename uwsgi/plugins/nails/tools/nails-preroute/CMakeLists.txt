set(LLVM_LINK_COMPONENTS support)

add_clang_executable(nails-preroute
  nails-preroute.cc
)

target_link_libraries(nails-preroute
  clangTooling
  clangBasic
  clangASTMatchers
)


