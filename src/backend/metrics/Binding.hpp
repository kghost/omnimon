#pragma once

#include <functional>
#include <memory>
#include <set>

namespace backend::metrics {

class Publisher;
class SubscriberBase;

class Publisher {
public:
  virtual ~Publisher() = default;

  void Notify();
  void AddSubscribe(SubscriberBase& subscriber) { _Subscribers.insert(std::ref(subscriber)); }
  void RemoveSubscribe(SubscriberBase& subscriber) { _Subscribers.erase(std::ref(subscriber)); }

private:
  class Comp {
  public:
    bool operator()(std::reference_wrapper<SubscriberBase> a, std::reference_wrapper<SubscriberBase> b) const {
      return std::addressof(a.get()) < std::addressof(b.get());
    }
  };
  std::set<std::reference_wrapper<SubscriberBase>, Comp> _Subscribers;
};

class SubscriberBase {
public:
  virtual ~SubscriberBase() = default;
  virtual void OnUpdate() = 0;
};

template <typename PublisherT, typename Callback>
  requires std::derived_from<PublisherT, Publisher> && std::invocable<Callback, std::shared_ptr<PublisherT>>
class SubscriberLambda : public SubscriberBase {
public:
  explicit SubscriberLambda(std::shared_ptr<PublisherT> publisher, Callback&& callback)
      : _Publisher(publisher), _Callback(std::move(callback)) {
    _Publisher->AddSubscribe(*this);
    OnUpdate();
  }
  ~SubscriberLambda() override { _Publisher->RemoveSubscribe(*this); }

  void OnUpdate() override { _Callback(_Publisher); }

private:
  const std::shared_ptr<PublisherT> _Publisher;
  Callback _Callback;
};

template <typename PublisherT, typename Callback>
  requires std::derived_from<PublisherT, Publisher> && std::invocable<Callback, std::shared_ptr<PublisherT>>
std::shared_ptr<SubscriberBase> MakeSubscriber(std::shared_ptr<PublisherT> publisher, Callback&& callback) {
  return std::make_shared<SubscriberLambda<PublisherT, Callback>>(publisher, std::forward<Callback>(callback));
}

} // namespace backend::metrics
