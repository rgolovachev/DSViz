#include "Observer/observer.h"

// Observable methods

namespace DSViz {

Observable::~Observable() {
  while (!list_observer_.empty()) {
    (*list_observer_.begin())->Unsubscribe();
  }
}

void Observable::Subscribe(IObserver *observer) {
  list_observer_.push_back(observer);
}

void Observable::Detach(IObserver *observer) {
  list_observer_.remove(observer);
}

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

Observer::Observer(IObservable *observable,
                   std::function<void(const std::any &)> lmbd)
    : observable_{observable}, lmbd_{lmbd} {
  if (observable_) {
    observable_->Subscribe(this);
  }
}

Observer::~Observer() {
  if (observable_) {
    observable_->Detach(this);
  }
}

void Observer::OnNotify(const std::any &msg) {
  msg_ = msg;
  lmbd_(msg_);
}

void Observer::Unsubscribe() {
  if (observable_) {
    observable_->Detach(this);
    observable_ = nullptr;
  }
}
} // namespace DSViz
