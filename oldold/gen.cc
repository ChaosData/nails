#include "gen.h"
#include "controller-helper.h"

#include "controller.h"

//extern ControllerMapper controller_mapper;
ControllerMapper controller_mapper;
ControllerFactoryMapper controller_factory_mapper;

TestController singleton_TestController;
Test2Controller singleton_Test2Controller;

void init_controllers() {
  controller_mapper.emplace(type_name(singleton_TestController), ControllerActionMapper());
  auto& tcam_TestController = controller_mapper.at(type_name(singleton_TestController));
  //tcam_TestController.c_ = new TestController(tcam_TestController);
  tcam_TestController.add("show", make_func(&TestController::show));
  tcam_TestController.add("index", make_func(&TestController::index));
  char const* const name = typeid(TestController).name();
  printf("name: %s (%p)\n", name, name);
  controller_factory_mapper.emplace(type_name(singleton_TestController), new ControllerFactory<TestController>());


  controller_mapper.emplace(type_name(singleton_Test2Controller), ControllerActionMapper());
  auto& tcam_Test2Controller = controller_mapper.at(type_name(singleton_Test2Controller));
  //tcam_Test2Controller.c_ = new Test2Controller(tcam_Test2Controller);
  tcam_Test2Controller.add("show", make_func(&Test2Controller::show));
  tcam_Test2Controller.add("destroy", make_func(&Test2Controller::destroy));
  char const* const name2 = typeid(Test2Controller).name();
  printf("name2: %s (%p)\n", name2, name2);

  controller_factory_mapper.emplace(type_name(singleton_Test2Controller), new ControllerFactory<Test2Controller>());

}

void teardown_controllers() {
  //auto& tcam_TestController = controller_mapper.at("TestController");
  //controller_mapper.erase("TestController");
  //delete tcam_TestController.c_;

  auto* cfm_TestController = controller_factory_mapper.at(type_name(singleton_TestController));
  controller_factory_mapper.erase(type_name(singleton_TestController));
  delete cfm_TestController;


  //auto& tcam_Test2Controller = controller_mapper.at("Test2Controller");
  //controller_mapper.erase("Test2Controller");
  //delete tcam_Test2Controller.c_;

  auto* cfm_Test2Controller = controller_factory_mapper.at(type_name(singleton_Test2Controller));
  controller_factory_mapper.erase(type_name(singleton_Test2Controller));
  delete cfm_Test2Controller;

}
