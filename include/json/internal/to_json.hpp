// Copyright (c) 2025 KiryuRS
// SPDX-License-Identifier: MIT

#pragma once

#include <concepts>
#include <string>

namespace json::internal {

template <typename T>
    requires (std::integral<T> || std::floating_point<T>)
auto to_json(T value)
{
    if constexpr (std::same_as<T, bool>)
    {
        return value ? "true" : "false";
    }
    else if constexpr (std::same_as<T, char>)
    {
        return std::string{'"'} + value + '"';
    }
    else
    {
        return value;
    }
}

template <std::convertible_to<std::string> T>
std::string to_json(const T& value)
{
    return std::string{'"'} + value + '"';
}

template <std::same_as<std::string_view> T>
std::string_view to_json(T value)
{
    return to_json(std::string{value});
}

} // namespace json::internal
