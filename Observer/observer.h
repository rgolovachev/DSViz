#ifndef OBSERVER_H
#define OBSERVER_H
#include <any>
#include <functional>
#include <list>

namespace DSViz {

class IObserver {
public:
  virtual ~IObserver() = default;
  virtual void OnNotify(const std::any &data) = 0;
  virtual void Unsubscribe() = 0;
};

class IObservable {
public:
  virtual ~IObservable() = default;
  virtual void Subscribe(IObserver *observer) = 0;
  virtual void Detach(IObserver *observer) = 0;
  virtual void Notify() = 0;
  virtual void Set(const std::any &msg) = 0;
};

class Observable : public IObservable {
public:
  Observable() = default;

  ~Observable() override;

  void Subscribe(IObserver *observer) override;

  void Detach(IObserver *observer) override;

  void Notify() override;

  void Set(const std::any &msg) override;

private:
  std::list<IObserver *> list_observer_;
  std::any msg_;
};

class Observer : public IObserver {
public:
  Observer(IObservable *observable, std::function<void(const std::any &)> lmbd);

  ~Observer() override;

  void OnNotify(const std::any &msg) override;

  void Unsubscribe() override;

private:
  std::any msg_;
  IObservable *observable_;
  std::function<void(const std::any &)> lmbd_;
};

} // namespace DSViz
#endif // OBSERVER_H
