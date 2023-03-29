#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "Common/query.h"
#include "Core/model.h"
#include "Observer/observer.h"
#include <any>
#include <functional>
#include <iostream>

namespace DSViz {

class Controller {
  using ArgsType = std::pair<int, int>;
  using UserQuery = UserQuery<int>;

public:
  Controller(IObservable *observable, Model *model);

  void Insert(const ArgsType &args);

  void Remove(const ArgsType &args);

  void Find(const ArgsType &args);

  void Split(const ArgsType &args);

  void Merge(const ArgsType &args);

  void DeleteTree(const ArgsType &args);

private:
  void HandleMsg(const UserQuery &data);
  auto GetCallback();

  Model *model_ptr_;
  std::unique_ptr<IObserver> port_in_;
};

} // namespace DSViz
#endif // CONTROLLER_H
