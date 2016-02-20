#pragma once

#include <unordered_map>
#include <experimental/optional>

#include "common.h"
#include "disposable.hpp"
#include "middleware.hpp"

namespace flow {

// store
template<class State>
class basic_store {
 public:
  using state_t = State;
  using action_t = action;

  basic_store() = delete;

  action_t dispatch(std::function<action_t()> action_creator) {
    return _dispatcher(action_creator());
  }

  action_t dispatch(action_t action) {
    return _dispatcher(action);
  }

  basic_disposable<> subscribe(state_subscribe_t <state_t> subscriber) const {
    subscriber(_current_state);
    return _subscribing(subscriber);
  }

  dispatch_t dispatch() const { return _dispatcher; }

  get_state_t<state_t> get_state() const { return [this]() { return _current_state; }; }

  subscribe_t<state_t> subscribe() const { return _subscribing; }

  state_t state() const { return _current_state; }

 private:

  static basic_store<state_t> create(const std::function<state_t(state_t, action)> reducer, const state_t initial_state,
                                     const std::experimental::optional<action> initial_action) {
    return basic_store<state_t>(reducer, initial_state);
  }

  basic_store(reducer_t <state_t> reducer, state_t initial_state, dispatch_t dispatcher)
      : _reducer(reducer), _current_state(initial_state), _dispatcher(dispatcher) {
    _subscribing = [&](state_subscribe_t<state_t> subscriber) -> basic_disposable<> {
      int id = _next_id++;
      _subscribers[id] = subscriber;

      return basic_disposable<>{
          disposable_holder{[this, id]() { return _subscribers.find(id) == std::end(_subscribers); },
                            [this, id]() { _subscribers.erase(id); }}};
    };
  }

  basic_store(reducer_t <state_t> reducer, state_t initial_state)
      : _reducer(reducer), _current_state(initial_state) {

    _dispatcher = [&](action_t action) -> action_t {

      if (_is_dispatching) {
        return action;
      }

      _is_dispatching = true;
      _current_state = _reducer(_current_state, action);
      _is_dispatching = false;

      std::for_each(std::begin(_subscribers),
                    std::end(_subscribers),
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

  std::function<state_t(state_t, action)> _reducer;
  state_t _current_state;
  int _next_id{0};
  std::unordered_map<int, std::function<void(state_t)>> _subscribers;
  bool _is_dispatching{false};

  dispatch_t _dispatcher;

  using disposed_t = std::function<bool()>;
  using disposable_t = std::function<void()>;

  struct disposable_holder {
    disposed_t disposed() const { return _disposed; }
    disposable_t disposable() const { return _disposer; }

    disposed_t _disposed;
    disposable_t _disposer;
  };

  subscribe_t <state_t> _subscribing;

  template <class S>
  friend basic_store<S> create_store(std::function<S(S, action)> reducer, const S &initial_state);

  template <class S>
  friend basic_store<S> create_store_with_action(std::function<S(S, action)> reducer, const S &initial_state,
                               const action &initial_action);

  template <class S>
  friend store_enhancer_t<S> apply_middleware(std::initializer_list<std::function<dispatch_transformer_t(basic_middleware<S>)>>
        transformers);

};


}  // namespace flow
