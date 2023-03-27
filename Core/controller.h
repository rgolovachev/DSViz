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
  Controller(IObservable *observable, Model<T> *model);

  void Insert(const ArgsType &args);

  void Remove(const ArgsType &args);

  void Find(const ArgsType &args);

  void Split(const ArgsType &args);

  void Merge(const ArgsType &args);

  void DeleteTree(const ArgsType &args);

private:
  void SetCallback(IObservable *observable);

  std::unique_ptr<IObserver> port_in_;
  Model<T> *model_ptr_;
};

template <typename T>
Controller<T>::Controller(IObservable *observable, Model<T> *model)
    : model_ptr_{model} {
  SetCallback(observable);
}

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

template <typename T> void Controller<T>::SetCallback(IObservable *observable) {
  auto lambda = [this](const std::any &msg_) {
    UserQuery<T> msg = std::any_cast<UserQuery<T>>(msg_);
    if (msg.type == QueryType::INSERT) {
      this->Insert(msg.args);
    } else if (msg.type == QueryType::REMOVE) {
      this->Remove(msg.args);
    } else if (msg.type == QueryType::FIND) {
      this->Find(msg.args);
    } else if (msg.type == QueryType::SPLIT) {
      this->Split(msg.args);
    } else if (msg.type == QueryType::MERGE) {
      this->Merge(msg.args);
    } else if (msg.type == QueryType::DELTREE) {
      this->DeleteTree(msg.args);
    }
  };
  port_in_ = std::move(std::make_unique<Observer>(observable, lambda));
}

} // namespace DSViz
#endif // CONTROLLER_H
