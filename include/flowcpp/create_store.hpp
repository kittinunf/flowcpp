#pragma once

#include <experimental/optional>
#include <unordered_map>

#include "store.hpp"

namespace flow {

namespace __internal {

template <class State>
basic_store<State> create_store(
    std::function<State(State, action)> reducer, State &&initial_state,
    std::experimental::optional<action> initial_action);
}

template <class State>
basic_store<State> create_store(
    std::function<State(State)> reducer, State &&initial_state) {
  return __internal::create_store(reducer, std::forward<State>(initial_state),
                                  std::experimental::optional<action>());
}

template <class State>
basic_store<State> create_store_with_action(
    std::function<State(State, action)> reducer, State &&initial_state,
    action &&initial_action) {
  return __internal::create_store(
      reducer, std::forward<State>(initial_state),
      std::experimental::make_optional(initial_action));
}

namespace __internal {
template <class State>
basic_store<State> create_store(
    std::function<State(State, action)> reducer, State &&initial_state,
    std::experimental::optional<action> initial_action) {
  using state_t = State;
  using action_t = action;

  // calculate state
  static state_t current_state = (initial_action)
                                     ? reducer(initial_state, *initial_action)
                                     : initial_state;

  static unsigned int next_id{0};

  static std::unordered_map<int, state_subscribe_t<state_t>> subscribers;

  static auto is_dispatching = false;

  std::function<action_t(action_t)> dispatch = [=](
      action_t action) -> action_t {

    if (is_dispatching) {
    }

    is_dispatching = true;
    current_state = reducer(current_state, action);
    is_dispatching = false;

    for_each(begin(subscribers), end(subscribers),
             [](const auto &pair) { pair.second(current_state); });

    return action;
  };

  std::function<state_t()> get_state = []() { return current_state; };

  using disposed_t = std::function<bool()>;
  using disposable_t = std::function<void()>;

  struct disposable_holder {
    disposed_t disposed() const { return _disposed; }
    disposable_t disposable() const { return _dispoable; }

    disposed_t _disposed;
    disposable_t _dispoable;
  };

  std::function<basic_disposable<>(state_subscribe_t<state_t>)> subscribe = [](
      std::function<void(state_t)> subscriber) -> basic_disposable<> {
    unsigned int id = next_id++;
    subscribers[id] = subscriber;
    return basic_disposable<>(disposable_holder{
        [=]() { return subscribers.find(id) == end(subscribers); },
        [=]() { subscribers.erase(id); }});
  };

  struct store_holder {
    dispatch_t dispatch() const { return _dispatch; }
    subscribe_t<state_t> subscribe() const { return _subscribe; }
    get_state_t<state_t> get_state() const { return _get_state; };

    dispatch_t _dispatch;
    subscribe_t<state_t> _subscribe;
    get_state_t<state_t> _get_state;
  };

  return basic_store<state_t>(
      store_holder{dispatch, subscribe, get_state});
}
}

}  // namespace flow
