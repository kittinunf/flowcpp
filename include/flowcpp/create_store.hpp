#pragma once

#include "store.hpp"

namespace flow {

template <class S>
basic_store<S> create_store(std::function<S(S, action)> reducer, const S& initial_state) {
  return basic_store<S>::create(reducer, initial_state, std::experimental::optional<action>());
}

template <class S>
basic_store<S> create_store_with_action(std::function<S(S, action)> reducer, const S& initial_state,
                                        const action& initial_action) {
  return basic_store<S>::create(reducer, initial_state, std::experimental::make_optional(initial_action));
}

}  // namespace flow
