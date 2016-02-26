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


void reselect_example() {
  std::cout << "Start: Selector example" << std::endl;

  struct State{
    int first_number;
    int second_number;
  };

  auto first_number_selector = flow::selector<State, int>{ [](auto state){
      return state.first_number;
    }};

  auto second_number_selector = flow::selector<State, int>{ [](auto state){
      return state.second_number;
    }};

  auto show_multiply_func = flow::result_func<std::string, int, int>{ [](auto params){
      auto a = std::get<0>(params);
      auto b = std::get<1>(params);
      return std::to_string(a) + " x " + std::to_string(b) + " = " + std::to_string(a*b);
    }};

  // Default memoize
  // auto int_equals = flow::equality_check<int>{ [](auto left, auto right){
  //   return left == right;
  // }};
  // auto equality_checks = std::make_tuple(int_equals, int_equals);
  // auto memoize = flow::default_memoize<std::string, int, int>(equality_checks);  

  // Map memoize
  auto int_key = flow::map_string_key<int>{ [](auto x){
    return std::to_string(x);
  }};
  auto keys = std::make_tuple(int_key, int_key);
  auto memoize = flow::map_memoize<std::string, int, int>(keys);

  //
  auto show_multiply_selector = 
          flow::create_selector(
            std::make_tuple(first_number_selector, second_number_selector), 
            show_multiply_func, 
            memoize);

  // Initialize State
  auto state = State();
  state.first_number = 2;
  state.second_number= 4;

  // Result
  auto result_1 = show_multiply_selector(state);
  std::cout << result_1 << std::endl;

  state.second_number = 5;
  auto result_2 = show_multiply_selector(state);
  std::cout << result_2 << std::endl;

  state.second_number = 5;
  auto result_3 = show_multiply_selector(state);
  std::cout << result_3 << std::endl;

  state.first_number = 10;
  auto result_4 = show_multiply_selector(state);
  std::cout << result_4 << std::endl;

  auto result_5 = show_multiply_selector(state);
  std::cout << result_5 << std::endl;

  state.first_number = 2;
  state.second_number = 4;
  auto result_6 = show_multiply_selector(state);
  std::cout << result_6 << std::endl;
}

void combine_selector(){
  std::cout << "Start: Combine selector example" << std::endl;

  struct SubState{
    int sub_id;
  };

  struct RootState{
    int id;
    SubState sub_state;
  };

  // Create First Selector
  auto id_selector = flow::selector<RootState, int>{[](auto state){ return state.id; }};
  auto sub_id_selector = flow::selector<RootState, int>{[](auto state){ return state.sub_state.sub_id; }};

  auto result_func = flow::result_func<std::string, int, int>{ [](auto params){
      auto id = std::get<0>(params);
      auto sub_id = std::get<1>(params);
      return "id = " + std::to_string(id) + ", sub_id = " + std::to_string(sub_id);
    }};

  auto int_key = flow::map_string_key<int>{ [](auto x){ return std::to_string(x); }};
  auto keys = std::make_tuple(int_key, int_key);
  auto result_func_memoize = flow::map_memoize<std::string, int, int>(keys);

  // Here is the first selector.
  // To combine multiple selector, we need to declare type explicitly.
  flow::selector<RootState, std::string> string_concat_selector = flow::create_selector(
                                                                      std::make_tuple(id_selector,sub_id_selector), 
                                                                      result_func, 
                                                                      result_func_memoize);

  // Create second selector which is combined from first selector.
  auto always_10_selector = flow::selector<RootState, int>{ [](auto _){ return 10; }};

  auto another_result_func = flow::result_func<int, std::string, int>{ [](auto params){ return std::get<1>(params); }};

  auto string_key = flow::map_string_key<std::string>{ [](auto x){ return x; }};
  auto second_keys = std::make_tuple(string_key, int_key);
  auto another_result_func_memoize = flow::map_memoize<int, std::string, int>(second_keys);

  // // Here is the second selector
  flow::selector<RootState, int> combined_selector = flow::create_selector(
                                                        std::make_tuple(string_concat_selector, always_10_selector), 
                                                        another_result_func, 
                                                        another_result_func_memoize);

  
  // Result
  auto root_state = RootState();
  root_state.id = 2;
  root_state.sub_state.sub_id = 4;

  auto result_1 = combined_selector(root_state);
  std::cout << result_1 << "\n\n";
}


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
  std::cout << "------------------------------" << std::endl;
  reselect_example();
  std::cout << "------------------------------" << std::endl;
  combine_selector();
  return 0;
}
