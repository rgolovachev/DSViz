#ifndef OBSERVER_H
#define OBSERVER_H
#include <list>

namespace DSViz {

// T - тип данных сообщения

template <typename T> class Observer;

template <typename T> class Observable {
public:
  Observable() = default;

  ~Observable() {
    while (!list_observer_.empty()) {
      list_observer_.front()->Unsubscribe();
    }
  }

  Observable(const Observable &) = delete;
  Observable &operator=(const Observable &) = delete;
  Observable(Observable &&) = delete;
  Observable &operator=(Observable &&) = delete;

  void Subscribe(Observer<T> *observer) {
    list_observer_.push_back(observer);
    observer->SetObservable(this);
    observer->OnNotify(msg_);
  }

  void Set(const T &msg) {
    msg_ = msg;
    Notify();
  }

  void Set(T &&msg) {
    msg_ = std::move(msg);
    Notify();
  }

  friend class Observer<T>;

private:
  void Detach(Observer<T> *observer) { list_observer_.remove(observer); }

  void Notify() const {
    for (auto observer : list_observer_) {
      observer->OnNotify(msg_);
    }
  }

  std::list<Observer<T> *> list_observer_;
  T msg_;
};

template <typename T> class Observer {
  using LambdaType = std::function<void(const T &)>;

public:
  Observer(LambdaType lmbd) : observable_{}, lmbd_{lmbd} {}

  ~Observer() { Unsubscribe(); }

  Observer(const Observer &) = delete;
  Observer &operator=(const Observer &) = delete;
  Observer(Observer &&) = delete;
  Observer &operator=(Observer &&) = delete;

  void Unsubscribe() {
    if (IsSubscribed()) {
      observable_->Detach(this);
      observable_ = nullptr;
    }
  }

  friend class Observable<T>;

private:
  bool IsSubscribed() const { return observable_; }

  void SetObservable(Observable<T> *observable) { observable_ = observable; }

  void OnNotify(const T &msg) const { lmbd_(msg); }

  Observable<T> *observable_;
  LambdaType lmbd_;
};

} // namespace DSViz
#endif // OBSERVER_H
