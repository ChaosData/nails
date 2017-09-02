#include "gen.h"
#include "controller-helper.h"
#include "router.h"

namespace nails {
  ControllerMapper controller_mapper;
  ControllerFactoryMapper controller_factory_mapper;
  ControllerNameMapper controller_name_mapper;
  ControllerFactorySingletonMapper controller_factory_singleton_mapper;
};

using namespace nails;

ChildExampleRouteSingleController singleton_ChildExampleRouteSingleController;
ExampleRouteSingleController singleton_ExampleRouteSingleController;
ExampleRouteSplitController singleton_ExampleRouteSplitController;
FooController singleton_FooController;
NewController singleton_NewController;
Test2Controller singleton_Test2Controller;
Test3Controller singleton_Test3Controller;
TestController singleton_TestController;


extern "C" void init_controllers() {
  factory_base* tmp_factory = nullptr;

  controller_mapper.emplace(type_name(singleton_ChildExampleRouteSingleController), ControllerActionMapper());
  tmp_factory = new ControllerFactory<ChildExampleRouteSingleController>();
  controller_factory_singleton_mapper.emplace(tmp_factory, &singleton_ChildExampleRouteSingleController);
  controller_factory_mapper.emplace(type_name(singleton_ChildExampleRouteSingleController), tmp_factory);
  controller_name_mapper.emplace("ChildExampleRouteSingle", type_name(singleton_ChildExampleRouteSingleController));
  auto& tcam_ChildExampleRouteSingleController = controller_mapper.at(type_name(singleton_ChildExampleRouteSingleController));
  tcam_ChildExampleRouteSingleController.add("child", make_func(&ChildExampleRouteSingleController::child));

  controller_mapper.emplace(type_name(singleton_ExampleRouteSingleController), ControllerActionMapper());
  tmp_factory = new ControllerFactory<ExampleRouteSingleController>();
  controller_factory_singleton_mapper.emplace(tmp_factory, &singleton_ExampleRouteSingleController);
  controller_factory_mapper.emplace(type_name(singleton_ExampleRouteSingleController), tmp_factory);
  controller_name_mapper.emplace("ExampleRouteSingle", type_name(singleton_ExampleRouteSingleController));
  auto& tcam_ExampleRouteSingleController = controller_mapper.at(type_name(singleton_ExampleRouteSingleController));
  tcam_ExampleRouteSingleController.add("fooer", make_func(&ExampleRouteSingleController::fooer));
  tcam_ExampleRouteSingleController.add("fooer2", make_func(&ExampleRouteSingleController::fooer2));

  controller_mapper.emplace(type_name(singleton_ExampleRouteSplitController), ControllerActionMapper());
  tmp_factory = new ControllerFactory<ExampleRouteSplitController>();
  controller_factory_singleton_mapper.emplace(tmp_factory, &singleton_ExampleRouteSplitController);
  controller_factory_mapper.emplace(type_name(singleton_ExampleRouteSplitController), tmp_factory);
  controller_name_mapper.emplace("ExampleRouteSplit", type_name(singleton_ExampleRouteSplitController));
  auto& tcam_ExampleRouteSplitController = controller_mapper.at(type_name(singleton_ExampleRouteSplitController));
  tcam_ExampleRouteSplitController.add("foo", make_func(&ExampleRouteSplitController::foo));

  controller_mapper.emplace(type_name(singleton_FooController), ControllerActionMapper());
  tmp_factory = new ControllerFactory<FooController>();
  controller_factory_singleton_mapper.emplace(tmp_factory, &singleton_FooController);
  controller_factory_mapper.emplace(type_name(singleton_FooController), tmp_factory);
  controller_name_mapper.emplace("Foo", type_name(singleton_FooController));
  auto& tcam_FooController = controller_mapper.at(type_name(singleton_FooController));
  tcam_FooController.add("foo", make_func(&FooController::foo));
  tcam_FooController.add("bar", make_func(&FooController::bar));

  controller_mapper.emplace(type_name(singleton_NewController), ControllerActionMapper());
  tmp_factory = new ControllerFactory<NewController>();
  controller_factory_singleton_mapper.emplace(tmp_factory, &singleton_NewController);
  controller_factory_mapper.emplace(type_name(singleton_NewController), tmp_factory);
  controller_name_mapper.emplace("New", type_name(singleton_NewController));
  auto& tcam_NewController = controller_mapper.at(type_name(singleton_NewController));

  controller_mapper.emplace(type_name(singleton_Test2Controller), ControllerActionMapper());
  tmp_factory = new ControllerFactory<Test2Controller>();
  controller_factory_singleton_mapper.emplace(tmp_factory, &singleton_Test2Controller);
  controller_factory_mapper.emplace(type_name(singleton_Test2Controller), tmp_factory);
  controller_name_mapper.emplace("Test2", type_name(singleton_Test2Controller));
  auto& tcam_Test2Controller = controller_mapper.at(type_name(singleton_Test2Controller));
  tcam_Test2Controller.add("show", make_func(&Test2Controller::show));
  tcam_Test2Controller.add("destroy", make_func(&Test2Controller::destroy));

  controller_mapper.emplace(type_name(singleton_Test3Controller), ControllerActionMapper());
  tmp_factory = new ControllerFactory<Test3Controller>();
  controller_factory_singleton_mapper.emplace(tmp_factory, &singleton_Test3Controller);
  controller_factory_mapper.emplace(type_name(singleton_Test3Controller), tmp_factory);
  controller_name_mapper.emplace("Test3", type_name(singleton_Test3Controller));
  auto& tcam_Test3Controller = controller_mapper.at(type_name(singleton_Test3Controller));
  tcam_Test3Controller.add("show", make_func(&Test3Controller::show));
  tcam_Test3Controller.add("index", make_func(&Test3Controller::index));

  controller_mapper.emplace(type_name(singleton_TestController), ControllerActionMapper());
  tmp_factory = new ControllerFactory<TestController>();
  controller_factory_singleton_mapper.emplace(tmp_factory, &singleton_TestController);
  controller_factory_mapper.emplace(type_name(singleton_TestController), tmp_factory);
  controller_name_mapper.emplace("Test", type_name(singleton_TestController));
  auto& tcam_TestController = controller_mapper.at(type_name(singleton_TestController));
  tcam_TestController.add("show", make_func(&TestController::show));
  tcam_TestController.add("index", make_func(&TestController::index));


  RouteNode::add("/new", {"GET"}, make_func(&NewController::index<url,methods>), type_name(singleton_NewController));
  RouteNode::add("/new/<id>", {"GET"}, make_func(&NewController::show<url,methods>), type_name(singleton_NewController));
  RouteNode::add("/new", {"POST"}, make_func(&NewController::create<url,methods>), type_name(singleton_NewController));
  RouteNode::add("/new/<id>", {"DELETE"}, make_func(&NewController::destroy<url,methods>), type_name(singleton_NewController));
  RouteNode::add("/custom/child/<string:id>", {"DELETE"}, make_func(&ChildExampleRouteSingleController::destroyer<url,methods>), type_name(singleton_ChildExampleRouteSingleController));
  RouteNode::add("/custom/<string:id>/destroy", {"DELETE", "POST"}, make_func(&ExampleRouteSingleController::destroyer<url,methods>), type_name(singleton_ExampleRouteSingleController));
  RouteNode::add("/custom", {"PUT"}, make_func(&ExampleRouteSingleController::putter<url,methods>), type_name(singleton_ExampleRouteSingleController));
  RouteNode::add("/custom", {"GET"}, make_func(&ExampleRouteSingleController::getter<url,methods>), type_name(singleton_ExampleRouteSingleController));
  RouteNode::add("/download", {"GET"}, make_func(&ExampleRouteSingleController::download<url,methods>), type_name(singleton_ExampleRouteSingleController));
  RouteNode::add("/cookie_test", {"GET"}, make_func(&ExampleRouteSingleController::cookie_test<url,methods>), type_name(singleton_ExampleRouteSingleController));
  RouteNode::add("/custom/<id>", {"GET"}, make_func(&ExampleRouteSplitController::show<url,methods>), type_name(singleton_ExampleRouteSplitController));
}

extern "C" void teardown_controllers() {

  auto* cfm_ChildExampleRouteSingleController = controller_factory_mapper.at(type_name(singleton_ChildExampleRouteSingleController));
  controller_factory_mapper.erase(type_name(singleton_ChildExampleRouteSingleController));
  delete cfm_ChildExampleRouteSingleController;

  auto* cfm_ExampleRouteSingleController = controller_factory_mapper.at(type_name(singleton_ExampleRouteSingleController));
  controller_factory_mapper.erase(type_name(singleton_ExampleRouteSingleController));
  delete cfm_ExampleRouteSingleController;

  auto* cfm_ExampleRouteSplitController = controller_factory_mapper.at(type_name(singleton_ExampleRouteSplitController));
  controller_factory_mapper.erase(type_name(singleton_ExampleRouteSplitController));
  delete cfm_ExampleRouteSplitController;

  auto* cfm_FooController = controller_factory_mapper.at(type_name(singleton_FooController));
  controller_factory_mapper.erase(type_name(singleton_FooController));
  delete cfm_FooController;

  auto* cfm_NewController = controller_factory_mapper.at(type_name(singleton_NewController));
  controller_factory_mapper.erase(type_name(singleton_NewController));
  delete cfm_NewController;

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
