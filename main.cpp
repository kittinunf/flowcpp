#include <iostream>
#include <vector>

#include <flowcpp/flow.h>

enum class counter_action_type {
  thunk,
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

std::string to_string(counter_action_type type) {
  switch (type) {
    case counter_action_type::increment:
      return "inc";
    case counter_action_type::decrement:
      return "dec";
    case counter_action_type::thunk:
      return "thunk";
  }
}

auto logging_middleware = [](flow::basic_middleware<counter_state>) {
  return [=](const flow::dispatch_t &next) {
    return [=](flow::action action) {
      auto next_action = next(action);
      std::cout << "after dispatch: " << to_string(action.type().as<counter_action_type>()) << std::endl;
      return next_action;
    };
  };
};

void simple_example() {
  std::cout << "Start: Simple example" << std::endl;

  auto store = flow::create_store_with_action<counter_state>(reducer, counter_state{}, increment_action{5});

  auto disposable = store.subscribe([](counter_state state) { std::cout << state.to_string() << std::endl; });

  store.dispatch(increment_action{2});
  store.dispatch(decrement_action{10});
  disposable.dispose();  // call dispose to stop notification prematurely
  store.dispatch(increment_action{3});
  store.dispatch(decrement_action{6});

  std::cout << "End: Simple example " << store.state().to_string() << std::endl;
}

void thunk_middleware_example() {
  std::cout << "Start: Thunk Middleware example" << std::endl;

  auto store = flow::apply_middleware<counter_state>(
      reducer, counter_state(), {flow::thunk_middleware<counter_state, counter_action_type>, logging_middleware});

  std::cout << store.state().to_string() << std::endl;

  store.dispatch(flow::thunk_action<counter_state, counter_action_type>{[&](auto dispatch, auto get_state) {
    dispatch(increment_action{1});
    dispatch(decrement_action{2});
    dispatch(increment_action{3});
  }});

  store.dispatch(flow::thunk_action<counter_state, counter_action_type>{[&](auto dispatch, auto get_state) {
    dispatch(increment_action{4});
    dispatch(decrement_action{5});
    dispatch(increment_action{6});
  }});

  std::cout << "End: Thunk Middleware example " << store.state().to_string() << std::endl;
}

int main() {
  simple_example();
  std::cout << "------------------------------" << std::endl;
  thunk_middleware_example();
  return 0;
}
