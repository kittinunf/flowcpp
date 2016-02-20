# flowcpp
Redux Implementation in C++14 - a predictable state container for C++.

# Introduction
flowcpp is a C++ header-only implementation of the JavaScript Redux library by Dan Abramov and the Community.
Idea is shamelessly borrowed from Redux's idea (All credits go to them). Implementation of flowcpp keeps as close as possible with original implemenation of Redux while maintaining C++ semantics.

For beginners, a complete and extensive walkthrough can be found in the official Redux repository. [Redux](https://github.com/rackt/redux)

# The Gist

A direct translation from original with slight modification [gist](https://github.com/rackt/redux/blob/master/README.md#the-gist)

``` C++
#include <flowcpp/flow.h>

enum class counter_action_type {
  increment,
  decrement,
};

struct increment_action {
  flow::any payload() const { return _payload; }
  flow::any type() const { return _type; }
  flow::any meta() const { return _meta; }
  bool error() const { return _error; }

  int _payload = {1};
  counter_action_type _type = {counter_action_type::increment};
  flow::any _meta;
  bool _error = false;
};

struct decrement_action {
  flow::any payload() const { return _payload; }
  flow::any type() const { return _type; }
  flow::any meta() const { return _meta; }
  bool error() const { return _error; }

  int _payload = {1};
  counter_action_type _type = {counter_action_type::decrement};
  flow::any _meta;
  bool _error = false;
};

struct counter_state {
  std::string to_string() { return "counter: " + std::to_string(_counter); }

  int _counter{0};
};

auto reducer = [](counter_state state, flow::action action) {
  int multiplier = 1;
  auto type = action.type().as<counter_action_type>();
  switch (type) {
    case counter_action_type::decrement:
      multiplier = -1;
      break;
    case counter_action_type::increment:
      multiplier = 1;
      break;
    default:
      break;
  }
  auto payload = action.payload().as<int>();
  state._counter += multiplier * payload;
  return state;
};

auto logging_middleware = [](flow::basic_middleware<counter_state>) {
  return [=](const flow::dispatch_t &next) {
    return [=](flow::action action) {
      std::cout << "before dispatch " << endl;
      auto next_action = next(action);
      std::cout << "after dispatch " << endl;
      return next_action;
    };
  };
};

int main() {
  auto store = flow::create_store_with_action<counter_state>(reducer, counter_state{}, increment_action{5});

  //for store with add-on middlewares
  auto store_with_middleware = flow::apply_middleware<counter_state>(
      reducer, counter_state(), {flow::thunk_middleware<counter_state, counter_action_type>});

  auto disposable = store.subscribe([](counter_state state) { std::cout << state.to_string() << std::endl; });

  store.dispatch(increment_action{2});
  store.dispatch(decrement_action{10});
  disposable.dispose();  // call dispose to stop notification prematurely
  store.dispatch(increment_action{3});
  store.dispatch(decrement_action{6});

  std::cout << store.state().to_string() << std::endl;
  return 0;
}
```

# Installation

* Include directory "flowcpp/include", then use umbrella header to access all files `#include <flowcpp/flow.h>` and you are done.
