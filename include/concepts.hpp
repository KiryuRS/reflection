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

template <typename T, typename RawT = std::remove_cvref_t<T>>
concept stringable = std::same_as<RawT, std::string_view> || std::same_as<std::string, RawT> || std::same_as<const char*, RawT>;

template <typename T>
concept enumerable = requires {
    requires std::is_enum_v<T>;
    { T::NONE } -> std::same_as<T>;
};

template <typename Functor, typename ArgT>
concept template_only_invocable = requires { std::declval<Functor>().template operator()<ArgT>(); };

template <typename Functor, typename ArgT>
concept template_invocable = requires { std::declval<Functor>().template operator()<ArgT>(ArgT{}); };

template <typename Functor, typename T>
concept any_invocable = template_only_invocable<Functor, T> || template_invocable<Functor, T> || std::invocable<Functor, T>;

template <typename T, typename RawT = std::remove_cvref_t<T>>
concept descriptor_like = requires {
    typename RawT::class_type;
    typename RawT::member_type;
    typename RawT::member_pointer_type;
    requires std::same_as<decltype(RawT::name), const std::string_view>;
    requires std::same_as<decltype(RawT::mem_type_str), const std::string_view>;
    requires std::same_as<decltype(RawT::mem_ptr), const typename T::member_pointer_type>;
};

template <typename T, typename RawT = std::remove_cvref_t<T>>
concept reflectable = requires {
    { RawT::meta_info_array_as_id() } -> std::same_as<void(*)()>; // did you forget to use GENERATE_META_INFO / REFLECT macro?
};

template <typename T, typename RawT = std::remove_cvref_t<T>>
concept reflect_and_printable = requires {
    requires reflectable<RawT>;
    { to_string(std::declval<RawT>()) } -> std::same_as<std::string>; // did you forget to use REFLECT macro?
};

} // namespace reflect::concepts
