#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "Common/query.h"
#include "Core/model.h"
#include "Observer/observer.h"

namespace DSViz {

class Controller {
  using ArgsType = std::pair<int, int>;
  using UserQuery = UserQuery<int>;

  auto GetCallback();

public:
  Controller(Model *model);

  Observer<UserQuery> *GetPortIn();

private:
  void Insert(const ArgsType &args);

  void Remove(const ArgsType &args);

  void Find(const ArgsType &args);

  void Split(const ArgsType &args);

  void Merge(const ArgsType &args);

  void DeleteTree(const ArgsType &args);

  void HandleMsg(const UserQuery &data);

  Model *model_ptr_;
  Observer<UserQuery> port_in_;
};

} // namespace DSViz
#endif // CONTROLLER_H
