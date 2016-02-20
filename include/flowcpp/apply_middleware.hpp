#pragma once

#include <initializer_list>
#include <numeric>
#include <vector>

#include "middleware.hpp"
#include "store.hpp"

namespace flow {

template <class S>
basic_store<S> apply_middleware(
    reducer_t<S> reducer, const S& state,
    std::initializer_list<std::function<dispatch_transformer_t(basic_middleware<S>)>> transformers) {
  using state_t = S;
  return basic_store<state_t>(reducer, state, transformers);
}

}  // namespace flow
