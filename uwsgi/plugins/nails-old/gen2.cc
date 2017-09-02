#include "gen.h"
#include "controller-helper.h"

#include "controller.h"

extern ControllerMapper controller_mapper;
void init_controllers() {
  controller_mapper.emplace("TestController", ControllerActionMapper());
  auto& tcam_TestController = controller_mapper.at("TestController");
  tcam_TestController.add("show", make_func(&TestController::show));
  tcam_TestController.add("index", make_func(&TestController::index));

  controller_mapper.emplace("Test2Controller", ControllerActionMapper());
  auto& tcam_Test2Controller = controller_mapper.at("Test2Controller");
  tcam_Test2Controller.add("show", make_func(&Test2Controller::show));
  tcam_Test2Controller.add("destroy", make_func(&Test2Controller::destroy));

}
