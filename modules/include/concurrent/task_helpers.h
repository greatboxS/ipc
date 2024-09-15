#ifndef TASK_HELPERS_H
#define TASK_HELPERS_H

#include <functional>
#include "task.h"

namespace ipc::core {

template <typename F>
struct function_traits;

template <typename R, typename... Args>
struct function_traits<R(Args...)> {
    using return_type = R;
    using argument_types = std::tuple<Args...>;

    static constexpr std::size_t arity = sizeof...(Args);

    template <std::size_t N>
    struct argument {
        using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
    };
};

template <typename R, typename... Args>
struct function_traits<R (*)(Args...)> {
    using return_type = R;
    using argument_types = std::tuple<Args...>;

    static constexpr std::size_t arity = sizeof...(Args);

    template <std::size_t N>
    struct argument {
        using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
    };
};

template <typename R, typename... Args>
struct function_traits<R (&)(Args...)> : function_traits<R(Args...)> {};

template <typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...)> : function_traits<R (*)(Args...)> {};

template <typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) const> : function_traits<R (*)(Args...)> {};

template <typename F>
struct function_traits<F &> : function_traits<F> {};

template <typename F>
struct function_traits : function_traits<decltype(&F::operator())> {};

template <typename F>
struct function_traits<const F &> : function_traits<F> {};

template <typename R, typename... Args>
struct function_traits<std::function<R(Args...)>> : function_traits<R(Args...)> {};

template <typename R, typename... Args>
struct function_traits<std::_Bind<R(Args...)>> {
    using return_type = typename function_traits<R>::return_type;
    using args_tuple = typename function_traits<R>::args_tuple;
};

template <typename F>
using function_return_type = typename function_traits<F>::return_type;

template <typename F>
using function_args_tuple = typename function_traits<F>::args_tuple;

} // namespace ipc::core

#endif // TASK_HELPERS_H