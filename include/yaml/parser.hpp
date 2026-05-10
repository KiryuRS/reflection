// Copyright (c) 2025 KiryuRS
// SPDX-License-Identifier: MIT

#pragma once

#include "convert.hpp"

#include <sstream>

namespace krrs::yaml {

template <krrs::reflect::concepts::reflectable T>
T deserialize(const std::string& yaml_config_str)
{
    constexpr std::string_view type_name = ::krrs::reflect::utility::get_short_name<T>();
    const YAML::Node node = YAML::Load(yaml_config_str);
    return node[type_name].as<T>();
}

template <krrs::reflect::concepts::reflectable T>
std::string serialize(const T& obj)
{
    constexpr std::string_view type_name = ::krrs::reflect::utility::get_short_name<T>();
    YAML::Node node;
    node[type_name] = obj;
    std::ostringstream oss;
    oss << node;
    return oss.str();
}

} // namespace krrs::yaml
