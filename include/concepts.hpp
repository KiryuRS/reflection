#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <string_view>
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

template <typename T, typename RawT = std::remove_cvref_t<T>>
concept same_as_array_type = detail::is_array_type<RawT>::value;

template <typename Functor, typename ArgT>
concept template_only_invocable = requires { std::declval<Functor>().template operator()<ArgT>(); };

template <typename Functor, typename ArgT>
concept template_invocable = requires { std::declval<Functor>().template operator()<ArgT>(ArgT{}); };

template <typename Functor, typename T>
concept any_invocable = template_only_invocable<Functor, T> || template_invocable<Functor, T> || std::invocable<Functor, T>;

template <typename T>
concept descriptor_like = requires {
    typename T::class_type;
    typename T::member_type;
    typename T::member_pointer_type;
    requires std::same_as<std::remove_cvref_t<decltype(T::name)>, std::string_view>;
    requires std::same_as<std::remove_cvref_t<decltype(T::mem_type_str)>, std::string_view>;
    requires std::same_as<std::remove_cvref_t<decltype(T::mem_ptr)>, typename T::member_pointer_type>;
};

template <typename T, typename RawT = std::remove_cvref_t<T>>
concept reflectable = requires {
    { RawT::meta_info_array_as_id() } -> std::same_as<void(*)()>;
};

template <typename T, typename RawT = std::remove_cvref_t<T>>
concept reflect_and_printable = requires {
    requires reflectable<RawT>;
    { to_string(std::declval<RawT>()) } -> std::same_as<std::string>;
};

} // namespace reflect::concepts
