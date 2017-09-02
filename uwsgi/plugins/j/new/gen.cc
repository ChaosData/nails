#include "gen.h"
#include "controller-helper.h"

ControllerMapper controller_mapper;
ControllerFactoryMapper controller_factory_mapper;

TestController singleton_TestController;
Test2Controller singleton_Test2Controller;
FooController singleton_FooController;

ControllerNameMapper controller_name_mapper;

extern "C" void init_controllers() {

  controller_mapper.emplace(type_name(singleton_TestController), ControllerActionMapper());
  controller_factory_mapper.emplace(type_name(singleton_TestController), new ControllerFactory<TestController>());
  controller_name_mapper.emplace("Test", type_name(singleton_TestController));
  auto& tcam_TestController = controller_mapper.at(type_name(singleton_TestController));
  tcam_TestController.add("show", make_func(&TestController::show));
  tcam_TestController.add("index", make_func(&TestController::index));

  controller_mapper.emplace(type_name(singleton_Test2Controller), ControllerActionMapper());
  controller_factory_mapper.emplace(type_name(singleton_Test2Controller), new ControllerFactory<Test2Controller>());
  controller_name_mapper.emplace("Test2", type_name(singleton_Test2Controller));
  auto& tcam_Test2Controller = controller_mapper.at(type_name(singleton_Test2Controller));
  tcam_Test2Controller.add("show", make_func(&Test2Controller::show));
  tcam_Test2Controller.add("destroy", make_func(&Test2Controller::destroy));

  controller_mapper.emplace(type_name(singleton_FooController), ControllerActionMapper());
  controller_factory_mapper.emplace(type_name(singleton_FooController), new ControllerFactory<FooController>());
  controller_name_mapper.emplace("Foo", type_name(singleton_FooController));
  auto& tcam_FooController = controller_mapper.at(type_name(singleton_FooController));
  tcam_FooController.add("foo", make_func(&FooController::foo));
  tcam_FooController.add("bar", make_func(&FooController::bar));

}

extern "C" void teardown_controllers() {

  auto* cfm_TestController = controller_factory_mapper.at(type_name(singleton_TestController));
  controller_factory_mapper.erase(type_name(singleton_TestController));
  delete cfm_TestController;

  auto* cfm_Test2Controller = controller_factory_mapper.at(type_name(singleton_Test2Controller));
  controller_factory_mapper.erase(type_name(singleton_Test2Controller));
  delete cfm_Test2Controller;

  auto* cfm_FooController = controller_factory_mapper.at(type_name(singleton_FooController));
  controller_factory_mapper.erase(type_name(singleton_FooController));
  delete cfm_FooController;

}
