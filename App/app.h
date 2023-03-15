#ifndef APP_H
#define APP_H
#include "App/mainwindow.h"
#include "Core/controller.h"
#include "Core/model.h"
#include "Core/view.h"

namespace DSViz {

template <typename T> class App {
public:
  App(const MainWindow *MW)
      : model_{}, view_{model_.GetPort(), MW}, controller_{view_.GetPort(),
                                                           &model_} {}

private:
  Model<T> model_;
  View<T> view_;
  Controller<T> controller_;
};

} // namespace DSViz

#endif // APP_H
