#pragma once

namespace flow {

template <class State>
using thunk_t = std::function<void(const flow::dispatch_t, const flow::get_state_t<State>)>;

template <class State, class ActionType>
auto thunk_middleware = [](flow::basic_middleware<State> middleware) {
  return [=](const flow::dispatch_t &dispatch) {
    return [=](flow::action action) -> flow::action {
      auto type = action.type().as<ActionType>();
      if (type == ActionType::thunk) {
        auto payload = action.payload().as<thunk_t<State>>();
        payload(dispatch, middleware.get_state());
      }
      return dispatch(action);
    };
  };
};

template <class State, class ActionType>
struct thunk_action {
  flow::any payload() const { return _payload; }
  flow::any type() const { return _type; }
  flow::any meta() const { return _meta; }
  bool error() const { return _error; }

  thunk_t<State> _payload;
  ActionType _type{ActionType::thunk};
  flow::any _meta;
  bool _error = false;
};

}  // namespace flow
