#ifndef APP_H
#define APP_H
#include "Core/controller.h"
#include "Core/model.h"
#include "Core/view.h"

namespace DSViz {

class App {
public:
  App();

private:
  void ConnectPorts();

  // я верю что это влезет на стек
  Model model_ = {};
  View view_ = {};
  Controller controller_;
};

} // namespace DSViz

#endif // APP_H
