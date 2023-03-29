#include "Observer/observer.h"

// Observable methods

namespace DSViz {

Observable::~Observable() {
  while (!list_observer_.empty()) {
    (*list_observer_.begin())->Unsubscribe(false);
    list_observer_.pop_front();
  }
}

void Observable::Subscribe(Observer *observer) {
  list_observer_.push_back(observer);
  observer->OnNotify(msg_);
}

void Observable::Detach(Observer *observer) { list_observer_.remove(observer); }

void Observable::Notify() {
  for (auto observer : list_observer_) {
    observer->OnNotify(msg_);
  }
}

void Observable::Set(const std::any &msg) {
  msg_ = msg;
  Notify();
}

// Observer methods

Observer::Observer(std::function<void(const std::any &)> lmbd)
    : observable_{}, lmbd_{lmbd} {}

Observer::~Observer() {
  if (observable_) {
    observable_->Detach(this);
  }
}

void Observer::OnNotify(const std::any &msg) { lmbd_(msg); }

void Observer::Unsubscribe(bool do_detach) {
  if (observable_) {
    if (do_detach) {
      observable_->Detach(this);
    }
    observable_ = nullptr;
  }
}
} // namespace DSViz
