# flowcpp
Redux Implemenation in C++14 - a predictable state container for C++.

# Introduction
flowcpp is a C++ header-only implementation of the JavaScript Redux library by Dan Abramov and the Community.
Idea is shamelessly borrowed from Redux's idea (All credits go to them). Implementation of flowcpp keeps as close as possible with original implemenation of Redux while maintaining C++ semantics.

For beginners, a complete and extensive walkthrough can be found in the official Redux repository. [Redux](https://github.com/rackt/redux)

# The Gist

A direct translation from original with slight modification [gist](https://github.com/rackt/redux/blob/master/README.md#the-gist)

``` C++
#include <iostream>

#include <flowcpp/flow.h>

enum class counter_action_type {
  nothing, increment, decrement
};

struct increment_action {
  int payload() const { return _payload; }

  counter_action_type type() const { return _type; }

  void *meta() const { return _meta; }

  bool error() const { return _error; }

  int _payload = {1};
  counter_action_type _type = {counter_action_type::increment};
  void *_meta = nullptr;
  bool _error = false;
};

struct decrement_action {
  int payload() const { return _payload; }

  counter_action_type type() const { return _type; }

  void *meta() const { return _meta; }

  bool error() const { return _error; }

  int _payload = {1};
  counter_action_type _type = {counter_action_type::decrement};
  void *_meta = nullptr;
  bool _error = false;
};

struct counter_state {
  std::string to_string() {
    return "counter: " + std::to_string(_counter);
  }

  int _counter{0};
};

using counter_action = flow::basic_action<int, counter_action_type, void *>;

auto reducer = [](counter_state state, counter_action action) -> counter_state {
  int multiplier = 1;
  switch (action.type()) {
    case counter_action_type::decrement:
      multiplier = -1;
      break;
    case counter_action_type::increment:
      multiplier = 1;
      break;
    case counter_action_type::nothing:
      break;
  }
  state._counter += multiplier * action.payload();
  return state;
};

int main() {

  // store
  // create store with possible initial action (+5)
  auto s = flow::create_store_with_action<counter_state, counter_action>(
      reducer, counter_state(), increment_action{5});

  // disposable
  // subscribe from store to get changes
  auto d = s.subscribe(
      [](counter_state state) { std::cout << state.to_string() << std::endl; });

  // dispatch to reducer in order to create new modified state from original state
  s.dispatch(increment_action{2});

  // invoke disposable to stop notification from store
  d.disposable()();

  // dispatch more actions
  s.dispatch(decrement_action{10});
  s.dispatch(increment_action{3});
  s.dispatch(decrement_action{5});

  // get state after perform all actions (staring with 5 then + 2 - 10 + 3 - 5)
  std::cout << s.state().to_string() << std::endl; //print counter: -5

  return 0;
}
```

# Installation

* Include directory "flowcpp/include", then use umbrella header to access all files `#include <flowcpp/flow.h>` and you are done.
