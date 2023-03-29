#include "app.h"

namespace DSViz {

App::App() : controller_{&model_} { ConnectPorts(); }

void App::ConnectPorts() {
  model_.GetPortOut()->Subscribe(view_.GetPortIn());
  view_.GetPortOut()->Subscribe(controller_.GetPortIn());
}

} // namespace DSViz
