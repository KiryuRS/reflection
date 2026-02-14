#pragma once

#include <concepts>
#include <optional>
#include <type_traits>
#include <vector>

namespace argparse::concepts {

namespace detail {

template <typename T>
struct same_as_vector : std::false_type
{
};

template <typename ... Ts>
struct same_as_vector<std::vector<Ts...>> : std::true_type
{
};

template <typename T>
struct is_optional : std::false_type
{
};

template <typename... Args>
struct is_optional<std::optional<Args...>> : std::true_type
{
};

} // namespace detail

template <typename T>
concept same_as_vector = detail::same_as_vector<T>::value;

template <typename T>
concept same_as_optional = detail::is_optional<std::remove_cvref_t<T>>::value;

} // namespace argparse::concepts
