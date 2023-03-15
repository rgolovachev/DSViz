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
  Controller(IObservable *observable, Model<T> *model) : model_ptr_{model} {
    SetCallback(observable);
  }

  void Insert(const ArgsType &args) {
    model_ptr_->Insert(args.first, args.second);
  }

  void Remove(const ArgsType &args) {
    model_ptr_->Remove(args.first, args.second);
  }

  void Find(const ArgsType &args) {
    model_ptr_->ExistKey(args.first, args.second);
  }

  void Split(const ArgsType &args) {
    model_ptr_->Split(args.first, args.second);
  }

  void Merge(const ArgsType &args) {
    model_ptr_->Merge(args.first, args.second);
  }

  void DeleteTree(const ArgsType &args) { model_ptr_->DeleteTree(args.first); }

private:
  void SetCallback(IObservable *observable) {
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
    port_in_.reset(new Observer{observable, lambda});
  }

  std::unique_ptr<IObserver> port_in_;
  Model<T> *model_ptr_;
};

} // namespace DSViz
#endif // CONTROLLER_H
