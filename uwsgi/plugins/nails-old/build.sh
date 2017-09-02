clang++ -std=c++14 -stdlib=libc++ \
  -Wall -Wextra -pedantic -g \
  -shared -fPIC -o libcontroller.so \
  -Iincludes \
  src/*.cc gen/*.cc controllers/*.cc

#  gen.cc controller-helper.cc nails_route_annotations.cc Request.cc common.cc main.cc \
#  TestController.cc Test2Controller.cc FooController.cc ExampleRouteSplitController.cc
