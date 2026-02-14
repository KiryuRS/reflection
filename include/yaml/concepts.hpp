#pragma once

#include <concepts>
#include <optional>
#include <ranges>
#include <string_view>
#include <utility>

namespace reflect::yaml::concepts {

namespace detail {

template <typename T>
struct is_optional : std::false_type
{
};

template <typename... Args>
struct is_optional<std::optional<Args...>> : std::true_type
{
};

} // namespace detail

template <typename T, typename RawT = std::remove_cvref_t<T>>
concept container_like = requires(RawT)
{
    // though std::string is something like a container, but we don't really regard it as one
    // hence we should ensure that its not a std::string or std::string_view
    requires !std::same_as<std::string, RawT>;
    requires !std::same_as<std::string_view, RawT>;
    typename RawT::value_type;
    typename RawT::allocator_type;
    requires std::ranges::range<RawT>;
};

template <typename T>
concept same_as_optional = detail::is_optional<std::remove_cvref_t<T>>::value;

} // namespace reflect::yaml::concepts
