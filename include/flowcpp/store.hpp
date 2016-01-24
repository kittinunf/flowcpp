#pragma once

#include "common.h"
#include "disposable.hpp"

namespace flow {

// store
template <class State>
class basic_store {
 public:
  using state_t = State;
  using action_t = action;

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

  std::function<state_t()> get_state() const { return _p->get_state(); }

  dispatch_t dispatch() const { return _p->dispatch(); }

  subscribe_t<state_t> subscribe() const { return _p->subscribe(); }

  state_t state() const { return _p->get_state()(); }

 private:
  struct concept {
    virtual ~concept() = default;

    virtual concept *copy() const = 0;

    virtual get_state_t<state_t> get_state() const = 0;

    virtual dispatch_t dispatch() const = 0;

    virtual subscribe_t<state_t> subscribe() const = 0;
  };

  template <class T>
  struct concrete : public concept {
    explicit concrete(T t) : _t(std::move(t)) {}

    concept *copy() const override { return new concrete(*this); }

    get_state_t<state_t> get_state() const override { return _t.get_state(); }

    dispatch_t dispatch() const override { return _t.dispatch(); }

    subscribe_t<state_t> subscribe() const override { return _t.subscribe(); }

    T _t;
  };

  std::unique_ptr<concept> _p;
};

}  // namespace flow
