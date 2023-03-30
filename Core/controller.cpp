#include "controller.h"

namespace DSViz {

// если что, компилятор требует от меня определить GetCallback заранее т.к. тип
// возвращаемого значения должен быть выведен компилятором (auto)
auto Controller::GetCallback() {
  return [this](const UserQuery &msg) { HandleMsg(msg); };
}

Controller::Controller(Model *model)
    : model_ptr_{model}, port_in_{GetCallback()} {}

Observer<Controller::UserQuery> *Controller::GetPortIn() { return &port_in_; }

void Controller::Insert(const ArgsType &args) {
  model_ptr_->Insert(args.first, args.second);
}

void Controller::Remove(const ArgsType &args) {
  model_ptr_->Remove(args.first, args.second);
}

void Controller::Find(const ArgsType &args) {
  model_ptr_->ExistKey(args.first, args.second);
}

void Controller::Split(const ArgsType &args) {
  model_ptr_->Split(args.first, args.second);
}

void Controller::Merge(const ArgsType &args) {
  model_ptr_->Merge(args.first, args.second);
}

void Controller::DeleteTree(const ArgsType &args) {
  model_ptr_->DeleteTree(args.first);
}

void Controller::HandleMsg(const UserQuery &data) {
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

} // namespace DSViz
