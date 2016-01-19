#pragma once

#include "disposable.hpp"

namespace flow {

template <class State, class Action>
class basic_store {
 public:
  using state_t = State;
  using action_t = Action;

  template <class T>
  basic_store(const T &t) : _p(new concrete<T>(t)) {}

  basic_store(basic_store &&) = default;

  basic_store(const basic_store &store) : _p(store._p->copy()) {}

  basic_store &operator=(basic_store store) {
    _p = move(store._p);
    return *this;
  }

  basic_store &operator=(basic_store &&) = default;

  action_t dispatch(std::function<action_t()> action) {
    return _p->dispatch()(action());
  }

  action_t dispatch(action_t action) {
    return _p->dispatch()(action);
  }

  basic_disposable<> subscribe(state_subscribe_t<state_t> subscriber) {
    subscriber(_p->get_state()());
    return _p->subscribe()(subscriber);
  }

  template <class S, class A>
  friend std::function<S()> get_state(const basic_store<S, A> &store);

  template <class S, class A>
  friend std::function<A(A)> dispatch(const basic_store<S, A> &store);

  template <class S, class A>
  friend std::function<basic_disposable<>(std::function<void(S)>)> subscribe(
      const basic_store<S, A> &store);

 private:
  struct concept {
    virtual ~concept() = default;

    virtual concept *copy() const = 0;

    virtual get_state_t<state_t> get_state() const = 0;

    virtual dispatch_t<action_t> dispatch() const = 0;

    virtual subscribe_t<state_t> subscribe() const = 0;
  };

  template <class T>
  struct concrete : public concept {
    explicit concrete(T t) : _t(std::move(t)) {}

    concept *copy() const override { return new concrete(*this); }

    get_state_t<state_t> get_state() const override { return _t.get_state(); }

    dispatch_t<action_t> dispatch() const override { return _t.dispatch(); }

    subscribe_t<state_t> subscribe() const override { return _t.subscribe(); }

    T _t;
  };

  std::unique_ptr<concept> _p;
};

template <class S, class A>
std::function<S()> get_state(const basic_store<S, A> &store) {
  return store._p->get_state();
}

template <class S, class A>
std::function<A(A)> dispatch(const basic_store<S, A> &store) {
  return store._p->dispatch();
}

template <class S, class A>
std::function<basic_disposable<>(std::function<void(S)>)> subscribe(
    const basic_store<S, A> &store) {
  return store._p->subscribe();
}

}  // namespace flow
