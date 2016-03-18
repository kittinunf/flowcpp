#pragma once

#include <experimental/optional>
#include <unordered_map>

#include "common.h"
#include "disposable.hpp"
#include "middleware.hpp"

namespace flow {

// store
template <class State>
class basic_store {
 public:
  using state_t = State;
  using action_t = action;

  basic_store() = delete;

  action_t dispatch(std::function<action_t()> action_creator) const { return _dispatcher(action_creator()); }

  action_t dispatch(action_t action) const { return _dispatcher(action); }

  basic_disposable<> subscribe(state_subscribe_t<state_t> subscriber, bool needs_cache = true) const {
    if (needs_cache) subscriber(_current_state);
    return _subscribing(subscriber);
  }

  state_t state() const { return _current_state; }

  template <class S>
  friend basic_store<S> create_store(std::function<S(S, action)> reducer, const S &initial_state);

  template <class S>
  friend basic_store<S> create_store_with_action(std::function<S(S, action)> reducer, const S &initial_state,
                                                 const action &initial_action);

  template <class S>
  friend basic_store<S> apply_middleware(
      reducer_t<S> reducer, const S &state,
      std::initializer_list<std::function<dispatch_transformer_t(basic_middleware<S>)>> transformers);

 private:
  static basic_store<state_t> create(const std::function<state_t(state_t, action)> reducer, const state_t initial_state,
                                     const std::experimental::optional<action> initial_action) {
    auto state = (initial_action) ? reducer(initial_state, *initial_action) : initial_state;
    return basic_store<state_t>(reducer, state);
  }

  basic_store(reducer_t<state_t> reducer, state_t initial_state) : _reducer(reducer), _current_state(initial_state) {
    _dispatcher = [&](action_t action) -> action_t {

      if (_is_dispatching) {
        return action;
      }

      _is_dispatching = true;
      _current_state = _reducer(_current_state, action);
      _is_dispatching = false;

      std::for_each(std::begin(_subscribers), std::end(_subscribers),
                    [&](const auto &pair) { pair.second(_current_state); });
      return action;
    };

    _subscribing = [&](state_subscribe_t<state_t> subscriber) -> basic_disposable<> {
      int id = _next_id++;
      _subscribers[id] = subscriber;

      return basic_disposable<>{
          disposable_holder{[this, id]() { return _subscribers.find(id) == std::end(_subscribers); },
                            [this, id]() { _subscribers.erase(id); }}};
    };
  }

  basic_store(reducer_t<state_t> reducer, state_t initial_state,
              std::initializer_list<std::function<dispatch_transformer_t(basic_middleware<state_t>)>> transformer)
      : basic_store(reducer, initial_state) {
    _dispatcher = std::accumulate(
        std::begin(transformer), std::end(transformer), _dispatcher, [&](dispatch_t acc, auto f) -> dispatch_t {
          auto middleware = basic_middleware<state_t>{middleware_holder{_dispatcher, [&]() { return _current_state; }}};

          auto dispatch_transformer = f(middleware);

          return dispatch_transformer(acc);
        });
  }

  std::function<state_t(state_t, action)> _reducer;
  state_t _current_state;
  int _next_id{0};
  std::unordered_map<int, state_subscribe_t<state_t>> _subscribers;
  bool _is_dispatching{false};

  dispatch_t _dispatcher;
  subscribe_t<state_t> _subscribing;

  // helpers
  struct disposable_holder {
    basic_disposable<>::disposed_t disposed() const { return _disposed; }
    basic_disposable<>::disposable_t disposable() const { return _disposer; }

    basic_disposable<>::disposed_t _disposed;
    basic_disposable<>::disposable_t _disposer;
  };

  struct middleware_holder {
    dispatch_t dispatch() const { return _dispatch; }
    get_state_t<state_t> get_state() const { return _get_state; }

    dispatch_t _dispatch;
    get_state_t<state_t> _get_state;
  };
};

}  // namespace flow
