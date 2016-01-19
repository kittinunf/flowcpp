#pragma once

#include <functional>

namespace flow {

template <class State, class Action>
class basic_store;

template <class Disposed = std::function<bool()>,
          class Disposable = std::function<void()>>
class basic_disposable;

template <class State, class Action>
using reducer_t = std::function<State(State, Action)>;

template <class State, class Action>
using store_creator_t =
    std::function<basic_store<State, Action>(reducer_t<State, Action>, State)>;

template <class State, class Action>
using store_enhancer_t = std::function<store_creator_t<State, Action>(
    store_creator_t<State, Action>)>;

template <class Action>
using dispatch_t = std::function<Action(Action)>;

template <class Action>
using dispatch_transformer_t =
    std::function<dispatch_t<Action>(dispatch_t<Action>)>;

template <class State>
using get_state_t = std::function<State()>;

template <class State>
using state_subscribe_t = std::function<void(State)>;

template <class State>
using subscribe_t = std::function<basic_disposable<>(state_subscribe_t<State>)>;

}  // namespace flow
