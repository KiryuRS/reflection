#pragma once

#include "../reflect/concepts.hpp"
#include "concepts.hpp"

#include <ranges>
#include <optional>
#include <sstream>
#include <string_view>
#include <vector>

namespace argparse::convertors {

template <typename T>
constexpr std::optional<T> parse_value(std::string_view)
{
    static_assert(false, "type not implemented!");
    return {};
}

template <typename T>
    requires (std::integral<T> || std::floating_point<T>)
constexpr std::optional<T> parse_value(std::string_view value)
{
    T val{};
    std::istringstream iss{std::string{value}}; // C++26 allows for implicit conversion
    // TODO: float / int are compatible, do we want to restrict this?
    if (!(iss >> val))
    {
        return std::nullopt;
    }
    return val;
}

template <>
constexpr std::optional<bool> parse_value(std::string_view value)
{
    const std::string lower = value
                            | std::views::transform([] (char c) -> char {
                                return static_cast<char>(std::tolower(c));
                              })
                            | std::ranges::to<std::string>();
    if (lower == "true")
        return true;

    if (lower == "false")
        return false;

    return std::nullopt;
}

template <::reflect::concepts::stringable T>
constexpr std::optional<T> parse_value(std::string_view value)
{
    return T{value};
}

template <concepts::same_as_optional T>
constexpr T parse_value(std::string_view value)
{
    using value_type = typename T::value_type;
    return parse_value<value_type>(value);
}

template <concepts::same_as_vector T>
constexpr std::optional<T> parse_value(std::string_view value)
{
    using value_type = typename T::value_type;
    using namespace std::string_view_literals;
    value_type default_value{};

    // when parsing a list of values, for any invalid entries, return default value
    // but indicate that parsing has issues
    bool parsed_fail = false;
    auto parsed_values = value | std::views::split(","sv)
                               | std::views::transform([&parsed_fail, &default_value](auto subrange) -> value_type {
                                   std::string_view str{subrange.begin(), subrange.end()};
                                   auto val = parse_value<value_type>(str);
                                   parsed_fail |= !val.has_value();
                                   return val.value_or(default_value);
                                 })
                               | std::ranges::to<std::vector>();

    return !parsed_fail ? std::optional{parsed_values} : std::nullopt;
}

} // namespace argparse::convertors
