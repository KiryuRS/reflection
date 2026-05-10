// Copyright (c) 2025 KiryuRS
// SPDX-License-Identifier: MIT

#pragma once

#include "../../include/reflect/reflect.hpp"
#include "concepts.hpp"
#include "internal/to_json.hpp"

#include <iomanip>

namespace json {

template <::reflect::concepts::reflectable T>
std::string convert_to_json(const T& obj)
{
    std::ostringstream oss;
    oss << '{';
    ::reflect::for_each<T>([&oss, &obj, obj_delimiter=""] <typename Descriptor>() mutable {
        using member_type = typename Descriptor::member_type;
        const auto& member = ::reflect::get_member_variable<Descriptor>(obj);
        oss << std::exchange(obj_delimiter, ", ") << std::quoted(Descriptor::name) << ": ";

        if constexpr (::reflect::concepts::reflectable<member_type>)
        {
            oss << convert_to_json<member_type>(member);
        }
        else if constexpr (concepts::same_as_vector<member_type>)
        {
            oss << '[';
            const char* delimiter = "";
            for (const auto& elem : member)
            {
                oss << std::exchange(delimiter, ", ") << internal::to_json(elem);
            }
            oss << ']';
        }
        else if constexpr (concepts::same_as_unordered_map<member_type>)
        {
            using key_type = typename member_type::key_type;
            static_assert(std::convertible_to<key_type, std::string>, "json serialization for unordered_map needs a string type for the key!");

            oss << '{';
            const char* delimiter = "";
            for (const auto& [key, value] :  member)
            {
                oss << std::exchange(delimiter, ", ") << internal::to_json(key) << ": " << internal::to_json(value);
            }
            oss << '}';
        }
        else if constexpr (concepts::same_as_optional<member_type>)
        {
            if (!member.has_value())
            {
                oss << "null";
            }
            else
            {
                oss << internal::to_json(member.value());
            }
        }
        else
        {
            oss << internal::to_json(member);
        }
    });
    oss << '}';
    return oss.str();
}

template <::reflect::concepts::reflectable T>
void convert_from_json(T&, const std::string&)
{
    // TODO: Implement me?
}

} // namespace json
