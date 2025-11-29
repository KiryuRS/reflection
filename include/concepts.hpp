#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <type_traits>

namespace reflect::concepts {

namespace detail {

template <typename T>
struct is_array_type : std::false_type
{
};

template <typename T, size_t N>
struct is_array_type<std::array<T, N>> : std::true_type
{
};

} // namespace detail

template <typename T>
concept always_false = false;

template <typename T, typename RawT = std::remove_cvref_t<T>>
concept same_as_array_type = detail::is_array_type<RawT>::value;

template <typename Functor, typename ArgT>
concept template_only_invocable = requires { std::declval<Functor>().template operator()<ArgT>(); };

template <typename Functor, typename ArgT>
concept template_invocable = requires { std::declval<Functor>().template operator()<ArgT>(ArgT{}); };

template <typename Functor, typename T>
concept any_invocable = template_only_invocable<Functor, T> || template_invocable<Functor, T> || std::invocable<Functor, T>;

} // namespace reflect::concepts
