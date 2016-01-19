#pragma once

#include "common.h"

namespace flow {

// middleware
template <class State, class Action>
class basic_middleware {
 public:
  using state_t = State;
  using action_t = Action;

  template <class T>
  basic_middleware(const T& t) : _p(new concrete<T>(t)) {}

  basic_middleware(basic_middleware&& middleware) = default;

  basic_middleware(const basic_middleware& middleware)
      : _p(middleware._p->copy()) {}

  basic_middleware& operator=(basic_middleware middleware) {
    _p = std::move(middleware._p);
    return *this;
  }

  basic_middleware& operator=(basic_middleware&&) = default;

  template <class S, class A>
  friend std::function<A(A)> dispatch(const basic_middleware<S, A>& middleware);

  template <class S, class A>
  friend std::function<S()> get_state(const basic_middleware<S, A>& middleware);

  template <class S, class A>
  friend S state(const basic_middleware<S, A>& middleware);

 private:
  struct concept {
    virtual ~concept() = default;

    virtual concept* copy() const = 0;

    virtual dispatch_t<action_t> dispatch() const = 0;

    virtual get_state_t<state_t> get_state() const = 0;
  };

  template <class T>
  struct concrete : public concept {
    explicit concrete(T t) : _t(std::move(t)) {}

    concept* copy() const override { return new concrete(*this); }

    dispatch_t<action_t> dispatch() const override { return _t.dispatch(); }

    get_state_t<state_t> get_state() const override { return _t.get_state(); }

    T _t;
  };

  std::unique_ptr<concept> _p;
};

// midddleware-based concept
template <class S, class A>
std::function<A(A)> dispatch(const basic_middleware<S, A>& middleware) {
  return middleware._p->dispatch();
}

template <class S, class A>
std::function<S()> get_state(const basic_middleware<S, A>& middleware) {
  return middleware._p->get_state();
}

template <class S, class A>
S state(const basic_middleware<S, A>& middleware) {
  return middleware._p->get_state()();
}

}  // namespace flow
