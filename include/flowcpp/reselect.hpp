#pragma once

// #define RESELECT_DEBUG

namespace flow{


	// Tuple iterator helper
	template <std::size_t I = 0, typename FuncT, typename... Tp>
	inline typename std::enable_if<I == sizeof...(Tp), void>::type
	  for_each_in_tuple(std::tuple<Tp...> &, FuncT) // Unused arguments are given no names.
	  { }

	template <std::size_t I = 0, typename FuncT, typename... Tp>
	inline typename std::enable_if<I < sizeof...(Tp), void>::type
	  for_each_in_tuple(std::tuple<Tp...>& t, FuncT f)
	  {
	    f(std::get<I>(t));
	    for_each_in_tuple<I + 1, FuncT, Tp...>(t, f);
	  }

	template <std::size_t I = 0, typename FuncT, typename... Tp, typename... Sp>
	inline typename std::enable_if<I == sizeof...(Tp), void>::type
	  for_each_in_tuple(std::tuple<Tp...> &, std::tuple<Sp...>& s, FuncT) // Unused arguments are given no names.
	  { }

	template <std::size_t I = 0, typename FuncT, typename... Tp, typename... Sp>
	inline typename std::enable_if<I < sizeof...(Tp), void>::type
	  for_each_in_tuple(std::tuple<Tp...>& t, std::tuple<Sp...>& s, FuncT f)
	  {
	    f(std::get<I>(t), std::get<I>(s));
	    for_each_in_tuple<I + 1, FuncT, Tp...>(t, s, f);
	  }

	template <std::size_t I = 0, typename FuncT, typename... Tp, typename... Sp, typename... Xp>
	inline typename std::enable_if<I == sizeof...(Tp), void>::type
	  for_each_in_tuple(std::tuple<Tp...> &, std::tuple<Sp...>& s, std::tuple<Xp...>& x, FuncT) // Unused arguments are given no names.
	  { }

	template <std::size_t I = 0, typename FuncT, typename... Tp, typename... Sp, typename... Xp>
	inline typename std::enable_if<I < sizeof...(Tp), void>::type
	  for_each_in_tuple(std::tuple<Tp...>& t, std::tuple<Sp...>& s, std::tuple<Xp...>& x, FuncT f)
	  {
	    f(std::get<I>(t), std::get<I>(s), std::get<I>(x));
	    for_each_in_tuple<I + 1, FuncT, Tp...>(t, s, x, f);
	  }


	// Helper function to create a tuple that has return type from a callable tuple.
	// From - http://stackoverflow.com/questions/35837126/create-a-tuple-from-the-results-of-a-callable-tuple/35837499
	template <typename... Funcs>
	auto tuple_function_ret_impl(std::tuple<Funcs...>) -> std::tuple<typename Funcs::result_type ...>;

	template <typename Tuple>
	using tuple_function_ret = decltype(tuple_function_ret_impl(Tuple()));
	//


	template <typename T, typename... Args>
	using result_func = std::function<T(std::tuple<Args...>)>;

	template <typename S, typename T>
	using selector = std::function<T(S)>;

	template <typename T, typename... Args>
	using memoize_func = std::function<T(std::tuple<Args...>)>;

	// Map memoize
	template <typename S>
	using map_string_key = std::function<std::string(S)>;

	template <typename T, typename... Args>
	auto map_memoize = [](auto keys){
	  return [=](result_func<T, Args...> func) -> memoize_func<T, Args...>{
	    auto result_map = std::unordered_map<std::string, T>{};
	    return [=](std::tuple<Args...> args) mutable -> T{

	      auto params_string = std::string();
	      for_each_in_tuple(keys, args, [&params_string = params_string](auto key_func, auto arg){
	        params_string = params_string + key_func(arg);
	      });

	      if (result_map.find(params_string) != result_map.end()){
					#ifdef RESELECT_DEBUG
	        std::cout << "use cache" << "\n";        
					#endif
	        return result_map[params_string];
	      }

	      auto new_result = func(args);
	      result_map[params_string] = new_result;
	      return new_result;
	    };
	  };
	};


	// Default memoize
	template <typename T>
	using equality_check = std::function<bool(T, T)>;

	template <typename T, typename... Args>
	auto default_memoize = [](auto equality_checks){
	  return [=](result_func<T, Args...> func) -> memoize_func<T, Args...>{

	    auto last_args = std::tuple<Args...>();
	    auto last_result = T();

	    return [=](std::tuple<Args...> args) mutable -> T{

	      auto all_args_are_equal = true;

	      for_each_in_tuple(args, last_args, equality_checks, [&all_args_are_equal = all_args_are_equal](auto arg, auto last_arg, auto equality_check){
	        auto arg_equal = equality_check(arg, last_arg);
	        if (!arg_equal){
	          all_args_are_equal = false; 
	        }
	      });

	      if (all_args_are_equal){ 
					#ifdef RESELECT_DEBUG
	        std::cout << "use cache" << "\n";
					#endif
	        return last_result; 
	      }

	      auto new_result = func(args);
	      last_args = args;
	      last_result = new_result;
	      return last_result;
	    };
	  };
	};

	// Helper function to assign `param[i] = selector[i](state);`
	template <std::size_t ...I, typename Params, typename Selectors, typename State>
	void copy_params_result_impl(Params &params, Selectors const & selectors, State const & state, std::index_sequence<I...>){
	    int dummy[] = { (std::get<I>(params) = std::get<I>(selectors)(state), 0)... };
	    static_cast<void>(dummy);
	}

	template <typename Params, typename Selectors, typename State>
	void copy_params_result(Params &params, Selectors const & selectors, State const & state){
	  auto index_sequence = std::make_index_sequence<std::tuple_size<Params>::value>();
	  copy_params_result_impl( params, selectors, state, index_sequence);
	}

	auto create_selector_creator = [](auto memoized_result_func){
	  return [=](auto selectors, auto func){
	    return [=](auto state){
	    	tuple_function_ret<decltype(selectors)> params;
        copy_params_result(params, selectors, state);
        return memoized_result_func(params);
	    };
	  };
	};

	auto create_selector = [](auto selectors, auto func, auto memoize){
	#ifdef RESELECT_DEBUG
	  auto memoized_result_func = memoize(
	    [func = func](auto args){
	      std::cout << "recompute" << std::endl;
	      return func(args);
	    });
	#else
	  auto memoized_result_func = memoize(func);
	#endif
		return create_selector_creator(memoized_result_func)(selectors, func);
	};


}
