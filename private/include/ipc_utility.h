#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <iostream>
#include <tuple>
#include <utility>
#include <functional>

namespace utility
{
    namespace detail
    {
        template <class F, class Tuple, std::size_t... Is>
        auto invoke_over_tuple(F &&f, Tuple &&tuple, std::index_sequence<Is...>) {
            using expand = int[];
            void(expand{0, ((f(std::get<Is>(std::forward<Tuple>(tuple)))), 0)...});
        }

        template <class Sequence, std::size_t I>
        struct append;
        template <std::size_t I, std::size_t... Is>
        struct append<std::index_sequence<Is...>, I> {
            using result = std::index_sequence<Is..., I>;
        };

        template <class Sequence>
        struct reverse;

        template <>
        struct reverse<std::index_sequence<>> {
            using type = std::index_sequence<>;
        };

        template <std::size_t I, std::size_t... Is>
        struct reverse<std::index_sequence<I, Is...>> {
            using subset = typename reverse<std::index_sequence<Is...>>::type;
            using type = typename append<subset, I>::result;
        };
    } // namespace detail

    template <class Sequence>
    using reverse = typename detail::reverse<Sequence>::type;

    template <class Tuple, class F>
    auto forward_over_tuple(F &&f, Tuple &&tuple) {
        using tuple_type = std::decay_t<Tuple>;
        constexpr auto size = std::tuple_size<tuple_type>::value;
        return detail::invoke_over_tuple(std::forward<F>(f),
                                         std::forward<Tuple>(tuple),
                                         std::make_index_sequence<size>());
    };

    template <class Tuple, class F>
    auto reverse_over_tuple(F &&f, Tuple &&tuple) {
        using tuple_type = std::decay_t<Tuple>;
        constexpr auto size = std::tuple_size<tuple_type>::value;
        return detail::invoke_over_tuple(std::forward<F>(f),
                                         std::forward<Tuple>(tuple),
                                         reverse<std::make_index_sequence<size>>());
    };

    template <typename Tuple, std::size_t... Is>
    static auto reverseTupleHelper(Tuple &&t, std::index_sequence<Is...>) {
        return std::make_tuple(std::get<sizeof...(Is) - 1 - Is>(std::forward<Tuple>(t))...);
    }

    template <typename... Args>
    static auto reverseTuple(std::tuple<Args...> &&t) {
        return reverseTupleHelper(std::move(t), std::index_sequence_for<Args...>{});
    }

    template <std::size_t I = 0, typename... Args>
    void freeTuple(std::tuple<Args...> &tuple) {
        if constexpr (I < sizeof...(Args)) {
            auto &element = std::get<I>(tuple);
            if constexpr (std::is_same_v<const char *, std::decay_t<decltype(element)>>) {
                delete[] const_cast<char *>(element);
            }
            freeTuple<I + 1>(tuple);
        }
    }

    template <typename... Args>
    struct TypeArray {
    };

    template <typename T, typename... Args>
    struct TypeArray<T, Args...> {
        static constexpr std::size_t count = sizeof...(Args) + 1;
        static constexpr std::size_t size = sizeof(T) + TypeArray<Args...>::size;
        static const char *names[count];
        static const std::type_info *types[count];
    };

    template <>
    struct TypeArray<> {
        static constexpr std::size_t count = 0;
        static constexpr std::size_t size = 0;
    };

    template <typename T, typename... Args>
    const char *TypeArray<T, Args...>::names[TypeArray<T, Args...>::count] = {typeid(T).name(), typeid(Args).name()...};

    template <typename T, typename... Args>
    const std::type_info *TypeArray<T, Args...>::types[TypeArray<T, Args...>::count] = {&typeid(T), &typeid(Args)...};

    template <typename T, typename... U>
    size_t getAddress(std::function<T(U...)> f) {
        typedef T(fnType)(U...);
        fnType **fnPointer = f.template target<fnType *>();
        return (size_t)*fnPointer;
    }

} // namespace utility
#endif // __UTILITY_H__