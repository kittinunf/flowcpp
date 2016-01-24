#pragma once

#include <functional>
#include "disposable.hpp"

namespace flow {

template <class State>
class basic_store;

template <class State>
using reducer_t = std::function<State(State, action)>;

template <class State>
using store_creator_t =
    std::function<basic_store<State>(reducer_t<State>, State)>;

template <class State>
using store_enhancer_t = std::function<store_creator_t<State>(
    store_creator_t<State>)>;

using dispatch_t = std::function<action(action)>;

using dispatch_transformer_t =
  std::function<dispatch_t(dispatch_t)>;

template <class State>
using get_state_t = std::function<State()>;

template <class State>
using state_subscribe_t = std::function<void(State)>;

template <class State>
using subscribe_t = std::function<basic_disposable<>(state_subscribe_t<State>)>;

}  // namespace flow
