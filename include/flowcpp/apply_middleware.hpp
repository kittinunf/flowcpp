#pragma once

#include <initializer_list>
#include <numeric>
#include <vector>

#include "middleware.hpp"
#include "store.hpp"

namespace flow {

template<class S>
store_enhancer_t<S> apply_middleware(std::initializer_list<std::function<dispatch_transformer_t(basic_middleware<S>)>>
                                     transformers) {

  using state_t = S;

  struct middleware_holder {
    dispatch_t dispatch() const { return _dispatch; }
    get_state_t<state_t> get_state() const { return _get_state; }

    dispatch_t _dispatch;
    get_state_t<state_t> _get_state;
  };

  return [=](store_creator_t<state_t> next) {
    return [=](reducer_t <state_t> reducer, state_t state) {

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

      return basic_store<state_t>(store._reducer, store.state(), new_dispatch);
    };
  };
}

}  // namespace flow
