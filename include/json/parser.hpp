// Copyright (c) 2025 KiryuRS
// SPDX-License-Identifier: MIT

#pragma once

#include "convert.hpp"

namespace krrs::json {

template <krrs::reflect::concepts::reflectable T>
T deserialize(const std::string&)
{
    static constexpr std::string_view class_name = ::krrs::reflect::utility::get_short_name<T>();
    T obj{};
    return {};
}

template <krrs::reflect::concepts::reflectable T>
std::string serialize(const T& obj)
{
    static constexpr std::string_view class_name = ::krrs::reflect::utility::get_short_name<T>();
    std::ostringstream oss;
    oss << '{' << std::quoted(class_name) << ": ";
    oss << convert_to_json(obj);
    oss << '}';
    return oss.str();
}

} // namespace krrs::json
