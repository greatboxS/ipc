#ifndef TASK_HELPERS_H
#define TASK_HELPERS_H

#include <functional>
#include "task.h"

namespace ipc::core {

template <typename F>
struct function_traits;

template <typename R, typename... Args>
struct function_traits<R (*)(Args...)> {
    using return_type = R;
    using args_tuple = std::tuple<Args...>;
};

template <typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...)> : function_traits<R (*)(Args...)> {};

template <typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) const> : function_traits<R (*)(Args...)> {};

template <typename F>
struct function_traits : function_traits<decltype(&F::operator())> {};

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
    using ReturnType = function_return_type<F>;
    return std::make_shared<task<ReturnType, std::decay_t<Args>...>>(std::forward<F>(func), std::move(callback), std::forward<Args>(args)...);
}

template <typename R, typename... Args>
auto make_task(R (*func)(Args...), std::function<void()> callback, Args &&...args) {
    return std::make_shared<task<R, std::decay_t<Args>...>>(func, std::move(callback), std::forward<Args>(args)...);
}

template <typename R, typename... Args>
auto make_task(std::_Bind<R(Args...)> func, std::function<void()> callback, Args &&...args) {
    using ReturnType = function_return_type<R>;
    return std::make_shared<task<ReturnType, std::decay_t<Args>...>>(std::move(func), std::move(callback), std::forward<Args>(args)...);
}

} // namespace ipc::core

#endif // TASK_HELPERS_H