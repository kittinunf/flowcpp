#pragma once

#include "common.h"

namespace flow {

// middleware
template <class State>
class basic_middleware {
 public:
  using state_t = State;
  using action_t = action;

  template <class T>
  basic_middleware(const T& t) : _p(new concrete<T>(t)) {}

  basic_middleware(basic_middleware&& middleware) = default;

  basic_middleware(const basic_middleware& middleware) : _p(middleware._p->copy()) {}

  basic_middleware& operator=(basic_middleware middleware) {
    _p = std::move(middleware._p);
    return *this;
  }

  basic_middleware& operator=(basic_middleware&&) = default;

  std::function<action_t(action_t)> dispatch() const { return _p->dispatch(); }

  std::function<state_t()> get_state() const { return _p->get_state(); }

  state_t state() const { return _p->get_state()(); }

 private:
  struct concept {
    virtual ~concept() = default;

    virtual concept* copy() const = 0;

    virtual dispatch_t dispatch() const = 0;

    virtual get_state_t<state_t> get_state() const = 0;
  };

  template <class T>
  struct concrete : public concept {
    explicit concrete(T t) : _t(std::move(t)) {}

    concept* copy() const override { return new concrete(*this); }

    dispatch_t dispatch() const override { return _t.dispatch(); }

    get_state_t<state_t> get_state() const override { return _t.get_state(); }

    T _t;
  };

  std::unique_ptr<concept> _p;
};

}  // namespace flow
