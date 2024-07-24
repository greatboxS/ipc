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

template <typename F, typename... Args>
auto make_task(F &&func, std::function<void()> callback, Args &&...args) {
    using ResultType = decltype(func(std::declval<Args>()...));
    using TaskType = task<ResultType, Args...>;
    return std::make_shared<TaskType>(std::forward<F>(func), std::move(callback), std::forward<Args>(args)...);
}

template <typename R, typename... Args>
auto make_task(R (*func)(Args...), std::function<void()> callback, Args &&...args) {
    return std::make_shared<task<R, std::decay_t<Args>...>>(func, std::move(callback), std::forward<Args>(args)...);
}

template <typename R, typename... Args, typename... CallArgs>
auto make_task(R (*func)(Args...), std::function<void()> callback, CallArgs &&...args) {
    return std::make_shared<task<R, std::decay_t<Args>...>>(func, std::move(callback), std::forward<Args>(args)...);
}

template <typename R, typename... Args>
auto make_task(std::_Bind<R(Args...)> func, std::function<void()> callback, Args &&...args) {
    using ReturnType = function_return_type<R>;
    return std::make_shared<task<ReturnType, std::decay_t<Args>...>>(std::move(func), std::move(callback), std::forward<Args>(args)...);
}

template <typename R, typename... Args, typename... CallArgs>
auto make_task(std::_Bind<R(Args...)> func, std::function<void()> callback, CallArgs &&...args) {
    using ReturnType = function_return_type<R>;
    return std::make_shared<task<ReturnType, std::decay_t<CallArgs>...>>(std::move(func), std::move(callback), std::forward<CallArgs>(args)...);
}

} // namespace ipc::core

#endif // TASK_HELPERS_H