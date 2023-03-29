#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "Common/query.h"
#include "Core/model.h"
#include "Observer/observer.h"
#include <any>
#include <functional>
#include <iostream>

namespace DSViz {

template <typename T> class Controller {
  using ArgsType = std::pair<int, T>;

public:
  Controller(IObservable *observable, Model *model);

  void Insert(const ArgsType &args);

  void Remove(const ArgsType &args);

  void Find(const ArgsType &args);

  void Split(const ArgsType &args);

  void Merge(const ArgsType &args);

  void DeleteTree(const ArgsType &args);

private:
  void HandleMsg(const UserQuery<T> &data);
  auto GetCallback();

  Model *model_ptr_;
  std::unique_ptr<IObserver> port_in_;
};

template <typename T>
Controller<T>::Controller(IObservable *observable, Model *model)
    : model_ptr_{model}, port_in_{std::make_unique<Observer>(observable,
                                                             GetCallback())} {}

template <typename T> void Controller<T>::Insert(const ArgsType &args) {
  model_ptr_->Insert(args.first, args.second);
}

template <typename T> void Controller<T>::Remove(const ArgsType &args) {
  model_ptr_->Remove(args.first, args.second);
}

template <typename T> void Controller<T>::Find(const ArgsType &args) {
  model_ptr_->ExistKey(args.first, args.second);
}

template <typename T> void Controller<T>::Split(const ArgsType &args) {
  model_ptr_->Split(args.first, args.second);
}

template <typename T> void Controller<T>::Merge(const ArgsType &args) {
  model_ptr_->Merge(args.first, args.second);
}

template <typename T> void Controller<T>::DeleteTree(const ArgsType &args) {
  model_ptr_->DeleteTree(args.first);
}

template <typename T> void Controller<T>::HandleMsg(const UserQuery<T> &data) {
  switch (data.type) {
  case QueryType::INSERT:
    Insert(data.args);
    break;
  case QueryType::REMOVE:
    Remove(data.args);
    break;
  case QueryType::FIND:
    Find(data.args);
    break;
  case QueryType::SPLIT:
    Split(data.args);
    break;
  case QueryType::MERGE:
    Merge(data.args);
    break;
  case QueryType::DELTREE:
    DeleteTree(data.args);
    break;
  default:
    break;
  }
}

template <typename T> auto Controller<T>::GetCallback() {
  return [this](const std::any &msg) {
    HandleMsg(std::any_cast<UserQuery<T>>(msg));
  };
}

} // namespace DSViz
#endif // CONTROLLER_H
