#include "gen.h"
#include "controller-helper.h"

ControllerMapper controller_mapper;
ControllerFactoryMapper controller_factory_mapper;

ExampleRouteSingleController singleton_ExampleRouteSingleController;

ControllerNameMapper controller_name_mapper;

extern "C" void init_controllers() {

  controller_mapper.emplace(type_name(singleton_ExampleRouteSingleController), ControllerActionMapper());
  controller_factory_mapper.emplace(type_name(singleton_ExampleRouteSingleController), new ControllerFactory<ExampleRouteSingleController>());
  controller_name_mapper.emplace("ExampleRouteSingle", type_name(singleton_ExampleRouteSingleController));
  auto& tcam_ExampleRouteSingleController = controller_mapper.at(type_name(singleton_ExampleRouteSingleController));
  tcam_ExampleRouteSingleController.add("fooer", make_func(&ExampleRouteSingleController::fooer));
  tcam_ExampleRouteSingleController.add("fooer2", make_func(&ExampleRouteSingleController::fooer2));

  controller_mapper.emplace(type_name(singleton_ExampleRouteSplitController), ControllerActionMapper());
  controller_factory_mapper.emplace(type_name(singleton_ExampleRouteSplitController), new ControllerFactory<ExampleRouteSplitController>());
  controller_name_mapper.emplace("ExampleRouteSplit", type_name(singleton_ExampleRouteSplitController));
  auto& tcam_ExampleRouteSplitController = controller_mapper.at(type_name(singleton_ExampleRouteSplitController));
  tcam_ExampleRouteSplitController.add("foo", make_func(&ExampleRouteSplitController::foo));

  controller_mapper.emplace(type_name(singleton_FooController), ControllerActionMapper());
  controller_factory_mapper.emplace(type_name(singleton_FooController), new ControllerFactory<FooController>());
  controller_name_mapper.emplace("Foo", type_name(singleton_FooController));
  auto& tcam_FooController = controller_mapper.at(type_name(singleton_FooController));
  tcam_FooController.add("foo", make_func(&FooController::foo));
  tcam_FooController.add("bar", make_func(&FooController::bar));

  controller_mapper.emplace(type_name(singleton_Test2Controller), ControllerActionMapper());
  controller_factory_mapper.emplace(type_name(singleton_Test2Controller), new ControllerFactory<Test2Controller>());
  controller_name_mapper.emplace("Test2", type_name(singleton_Test2Controller));
  auto& tcam_Test2Controller = controller_mapper.at(type_name(singleton_Test2Controller));
  tcam_Test2Controller.add("show", make_func(&Test2Controller::show));
  tcam_Test2Controller.add("destroy", make_func(&Test2Controller::destroy));

  controller_mapper.emplace(type_name(singleton_Test3Controller), ControllerActionMapper());
  controller_factory_mapper.emplace(type_name(singleton_Test3Controller), new ControllerFactory<Test3Controller>());
  controller_name_mapper.emplace("Test3", type_name(singleton_Test3Controller));
  auto& tcam_Test3Controller = controller_mapper.at(type_name(singleton_Test3Controller));
  tcam_Test3Controller.add("show", make_func(&Test3Controller::show));
  tcam_Test3Controller.add("index", make_func(&Test3Controller::index));

  controller_mapper.emplace(type_name(singleton_TestController), ControllerActionMapper());
  controller_factory_mapper.emplace(type_name(singleton_TestController), new ControllerFactory<TestController>());
  controller_name_mapper.emplace("Test", type_name(singleton_TestController));
  auto& tcam_TestController = controller_mapper.at(type_name(singleton_TestController));
  tcam_TestController.add("show", make_func(&TestController::show));
  tcam_TestController.add("index", make_func(&TestController::index));


  router.add("/custom/:id/destroy", {"DELETE", "POST"}, make_func(&ExampleRouteSingleController::destroyer<url,methods>), type_name(singleton_ExampleRouteSingleController));
  router.add("/custom/:id", {"GET"}, make_func(&ExampleRouteSplitController::show<url,methods>), type_name(singleton_ExampleRouteSplitController));
}

extern "C" void teardown_controllers() {

  auto* cfm_ExampleRouteSingleController = controller_factory_mapper.at(type_name(singleton_ExampleRouteSingleController));
  controller_factory_mapper.erase(type_name(singleton_ExampleRouteSingleController));
  delete cfm_ExampleRouteSingleController;

  auto* cfm_ExampleRouteSplitController = controller_factory_mapper.at(type_name(singleton_ExampleRouteSplitController));
  controller_factory_mapper.erase(type_name(singleton_ExampleRouteSplitController));
  delete cfm_ExampleRouteSplitController;

  auto* cfm_FooController = controller_factory_mapper.at(type_name(singleton_FooController));
  controller_factory_mapper.erase(type_name(singleton_FooController));
  delete cfm_FooController;

  auto* cfm_Test2Controller = controller_factory_mapper.at(type_name(singleton_Test2Controller));
  controller_factory_mapper.erase(type_name(singleton_Test2Controller));
  delete cfm_Test2Controller;

  auto* cfm_Test3Controller = controller_factory_mapper.at(type_name(singleton_Test3Controller));
  controller_factory_mapper.erase(type_name(singleton_Test3Controller));
  delete cfm_Test3Controller;

  auto* cfm_TestController = controller_factory_mapper.at(type_name(singleton_TestController));
  controller_factory_mapper.erase(type_name(singleton_TestController));
  delete cfm_TestController;

}
