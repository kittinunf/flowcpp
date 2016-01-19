# flowcpp
Redux Implemenation in C++14 - a predictable state container for C++.

# Introduction
flowcpp is a C++ header-only implementation of the JavaScript Redux library by Dan Abramov and the Community.
Idea is shamelessly borrowed from Redux's idea (All credits go to them). Implementation of flowcpp keeps as close as possible with original implemenation of Redux while maintaining C++ semantics.

For beginners, a complete and extensive walkthrough can be found [here] in the official Redux repository. [Redux](https://github.com/rackt/redux)

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
  decrement_action(int payload) : _payload{payload} { }

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
  switch (flow::type(action)) {
    case counter_action_type::decrement:
      multiplier = -1;
      break;
    case counter_action_type::increment:
      multiplier = 1;
      break;
    case counter_action_type::nothing:
      break;
  }
  state._counter += multiplier * flow::payload(action);
  return state;
};

// store
auto s = flow::create_store_with_action<counter_state, counter_action>(reducer, counter_state(), increment_action{5});

int main() {
  // disposable
  auto d = s.subscribe([](counter_state state) { std::cout << state.to_string() << std::endl; });
  
  // call dispatch to let reducer create new modified state from original state
  s.dispatch(increment_action{2});
  
  // call dispose to stop subscription, lambda in subscribe block is no longer functioning
  flow::disposable(d)();

  // dispatch more actions
  s.dispatch(decrement_action{10});
  s.dispatch(increment_action{3});
  s.dispatch(decrement_action{5});

  // get state after perform all actions
  
  // actions are starting with 5 then + 2 - 10 + 3 - 5
  std::cout << flow::get_state(s)().to_string() << std::endl; // print counter: -5
}
```

# Installation

* Include directory "flowcpp/include", then use umbrella header to access all files `#include <flowcpp/flow.h>` and you are done.
