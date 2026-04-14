#pragma once

#include <concepts>
#include <utility>

namespace reflect {

template <typename...>
struct typelist
{
};

namespace detail {

template <typename T>
struct same_as_typelist_impl : std::false_type
{
};

template <typename... Ts>
struct same_as_typelist_impl<typelist<Ts...>> : std::true_type
{
};

template <typename T>
struct typelist_size;

template <typename... Ts>
struct typelist_size<typelist<Ts...>>
{
    static constexpr std::size_t value = sizeof...(Ts);
};

template <std::size_t I, typename T>
struct typelist_element;

template <typename T, typename... Ts>
struct typelist_element<0, typelist<T, Ts...>>
{
    using type = T;
};

template <std::size_t I, typename T, typename... Ts>
struct typelist_element<I, typelist<T, Ts...>>
{
    using type = typename typelist_element<I - 1, typelist<Ts...>>::type;
};

} // namespace detail

template <typename T>
concept same_as_typelist = detail::same_as_typelist_impl<T>::value;

template <same_as_typelist Typelist>
inline constexpr auto typelist_size_v = detail::typelist_size<Typelist>::value;

template <std::size_t I, same_as_typelist Typelist>
using typelist_element_t = detail::typelist_element<I, Typelist>::type;

} // namespace reflect
