// Copyright (c) 2025 KiryuRS
// SPDX-License-Identifier: MIT

#pragma once

#include <concepts>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace json::concepts {

namespace detail {

template <typename T>
struct is_optional : std::false_type
{
};

template <typename... Args>
struct is_optional<std::optional<Args...>> : std::true_type
{
};

template <typename>
struct is_vector : std::false_type
{
};

template <typename ... Args>
struct is_vector<std::vector<Args...>> : std::true_type
{
};

template <typename>
struct is_unordered_map : std::false_type
{
};

template <typename ... Args>
struct is_unordered_map<std::unordered_map<Args...>> : std::true_type
{
};

} // namespace detail

template <typename T>
concept same_as_vector = detail::is_vector<std::remove_cvref_t<T>>::value;

template <typename T>
concept same_as_unordered_map = detail::is_unordered_map<std::remove_cvref_t<T>>::value;

template <typename T>
concept same_as_optional = detail::is_optional<std::remove_cvref_t<T>>::value;

} // namespace json::concepts
