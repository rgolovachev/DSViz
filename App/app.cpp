#include "app.h"

namespace DSViz {

App::App() : controller_{&model_} { ConnectPorts(); }

void App::ConnectPorts() {
  model_.SubscribeToBareTree(view_.GetPortIn());
  view_.SubscribeToUserInput(controller_.GetPortIn());
}

} // namespace DSViz
