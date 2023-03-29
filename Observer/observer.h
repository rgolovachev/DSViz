#ifndef OBSERVER_H
#define OBSERVER_H
#include <any>
#include <functional>
#include <list>

namespace DSViz {

class Observer;

class Observable {
public:
  Observable() = default;

  ~Observable();

  void Subscribe(Observer *observer);

  void Detach(Observer *observer);

  void Notify();

  void Set(const std::any &msg);

private:
  std::list<Observer *> list_observer_;
  std::any msg_;
};

class Observer {
  using LambdaType = std::function<void(const std::any &)>;

public:
  Observer(LambdaType lmbd);

  ~Observer();

  void OnNotify(const std::any &msg);

  void Unsubscribe(bool do_detach = true);

private:
  Observable *observable_;
  LambdaType lmbd_;
};

} // namespace DSViz
#endif // OBSERVER_H
