#ifndef OBSERVER_H
#define OBSERVER_H
#include <any>
#include <functional>
#include <list>

namespace DSViz {

// T - тип данных сообщения

template <typename T> class Observer;

template <typename T> class Observable {
public:
  Observable() = default;

  ~Observable() {
    while (!list_observer_.empty()) {
      (*list_observer_.begin())->Unsubscribe(false);
      list_observer_.pop_front();
    }
  }

  void Subscribe(Observer<T> *observer) {
    list_observer_.push_back(observer);
    observer->OnNotify(msg_);
  }

  void Detach(Observer<T> *observer) { list_observer_.remove(observer); }

  void Notify() {
    for (auto observer : list_observer_) {
      observer->OnNotify(msg_);
    }
  }

  void Set(const T &msg) {
    msg_ = msg;
    Notify();
  }

private:
  std::list<Observer<T> *> list_observer_;
  T msg_;
};

template <typename T> class Observer {
  using LambdaType = std::function<void(const T &)>;

public:
  Observer(LambdaType lmbd) : observable_{}, lmbd_{lmbd} {}

  ~Observer() {
    if (observable_) {
      observable_->Detach(this);
    }
  }

  void OnNotify(const T &msg) { lmbd_(msg); }

  void Unsubscribe(bool do_detach = true) {
    if (observable_) {
      if (do_detach) {
        observable_->Detach(this);
      }
      observable_ = nullptr;
    }
  }

private:
  Observable<T> *observable_;
  LambdaType lmbd_;
};

} // namespace DSViz
#endif // OBSERVER_H
