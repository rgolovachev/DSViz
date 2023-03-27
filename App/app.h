#ifndef APP_H
#define APP_H
#include "Core/controller.h"
#include "Core/model.h"
#include "Core/view.h"

namespace DSViz {

template <typename T> class App {
public:
  App() : view_{model_.GetPort()}, controller_{view_.GetPort(), &model_} {}

private:
  // 176 байт все поля весят, я думаю это приемлемо
  Model<T> model_ = {};
  View<T> view_;
  Controller<T> controller_;
};

} // namespace DSViz

#endif // APP_H
