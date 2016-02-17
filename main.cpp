#include <iostream>
#include <vector>

#include <flowcpp/flow.h>

enum class counter_action_type {
  nothing, thunk, increment, decrement,
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
  std::string to_string() {
    return "counter: " + std::to_string(_counter);
  }

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
  auto payload = action.payload().as<const int>();
  state._counter += multiplier * payload;
  return state;
};

auto logging_middleware = [](flow::basic_middleware<counter_state>) {
  return [=](const flow::dispatch_t& next) {
    return [=](flow::action action) {
      std::cout << "before dispatch" << std::endl;
      auto next_action = next(action);
      std::cout << "after dispatch" << std::endl;
      return next_action;
    };
  };
};

void simple_example(){
  std::cout << "simple example" << std::endl;

  auto store = flow::create_store_with_action<counter_state>(reducer, counter_state(), increment_action{10});
  
  // create store with middleware
  // auto store = flow::apply_middleware<counter_state>({logging_middleware})(
  // std::bind(flow::create_store<counter_state>, std::placeholders::_1, std::placeholders::_2))(reducer, counter_state());

  auto disposable = store.subscribe([](counter_state state) { std::cout << state.to_string() << std::endl; });

  store.dispatch(increment_action{2});

  store.dispatch(decrement_action{10});
  store.dispatch(increment_action{3});

  disposable.disposable()();
  store.dispatch(decrement_action{5});

  std::cout << "finish: " << store.state().to_string() << "\n\n";
}

void thunk_middleware_example(){
  std::cout << "thunk middleware example" << std::endl;

  auto store = flow::apply_middleware<counter_state>({flow::thunk_middleware<counter_state,counter_action_type>})
                (flow::create_store<counter_state>)
                  (reducer, counter_state());

  std::cout << store.state().to_string() << std::endl;

  store.dispatch(flow::thunk_action<counter_state,counter_action_type>{ 
    [&](auto dispatch, auto get_state){
        dispatch(increment_action{3});
        std::cout << get_state().to_string() << std::endl;
        dispatch(decrement_action{10});
        std::cout << get_state().to_string() << std::endl;
    }});
}

int main() {
  simple_example();
  thunk_middleware_example();
  return 0;
}


