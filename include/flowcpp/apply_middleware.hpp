#pragma once

#include <initializer_list>
#include <numeric>
#include <vector>

#include "middleware.hpp"
#include "store.hpp"

namespace flow {

template <class State>
store_enhancer_t<State> apply_middleware(
    std::initializer_list<std::function<
        dispatch_transformer_t(basic_middleware<State>)>>
        transformers) {
  using state_t = State;

  struct middleware_holder {
    dispatch_t dispatch() const { return _dispatch; }
    get_state_t<state_t> get_state() const { return _get_state; }

    dispatch_t _dispatch;
    get_state_t<state_t> _get_state;
  };

  struct store_holder {
    dispatch_t dispatch() const { return _dispatch; }
    subscribe_t<state_t> subscribe() const { return _subscribe; }
    get_state_t<state_t> get_state() const { return _get_state; };

    dispatch_t _dispatch;
    subscribe_t<state_t> _subscribe;
    get_state_t<state_t> _get_state;
  };

  return [=](store_creator_t<state_t> next) {
    return [=](reducer_t<state_t> reducer, state_t state) {

      auto store = next(reducer, state);

      auto middleware = basic_middleware<state_t>(
          middleware_holder{store.dispatch(), store.get_state()});

      std::vector<dispatch_transformer_t> chain;
      std::transform(begin(transformers), end(transformers),
                     std::back_inserter(chain),
                     [=](auto f) { return f(middleware); });

      auto new_dispatch = std::accumulate(
          rbegin(chain), rend(chain), store.dispatch(),
          [](dispatch_t arg, auto f) { return f(arg); });

      return basic_store<state_t>(
          store_holder{new_dispatch, store.subscribe(), store.get_state()});
    };
  };
}

}  // namespace flow
