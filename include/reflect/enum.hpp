#pragma once

#include "concepts.hpp"
#include "preprocessor.hpp"
#include "utility.hpp"

#include <sstream>
#include <format>

namespace reflect {

#define ENUM_ENUM_CASE(Enum, value) \
    case Enum::value:               \
        return PP_STRINGIZE(value);

#define ENUM_STR_CASE(Enum, value)                              \
    case ::reflect::utility::hash_dj2ba(PP_STRINGIZE(value)):   \
        return Enum::value;

#define ENUM_PRINTABLE(Enum, ...)                                               \
    inline constexpr std::string_view enum_to_string(Enum value)                \
    {                                                                           \
        switch (value)                                                          \
        {                                                                       \
        PP_FOR_EACH(ENUM_ENUM_CASE, Enum, PP_EVAL_TUPLE(__VA_ARGS__))           \
        default:                                                                \
            break;                                                              \
        }                                                                       \
        return "<unknown>";                                                     \
    }                                                                           \
                                                                                \
    Enum string_to_enum(Enum, const ::reflect::concepts::stringable auto& str)  \
    {                                                                           \
        static_assert(::reflect::concepts::enumerable<Enum>);                   \
        switch (::reflect::utility::hash_dj2ba(str))                            \
        {                                                                       \
        PP_FOR_EACH(ENUM_STR_CASE, Enum, PP_EVAL_TUPLE(__VA_ARGS__))            \
        default:                                                                \
            break;                                                              \
        }                                                                       \
        return Enum::NONE;                                                      \
    }                                                                           \
                                                                                \
    std::ostream& operator<<(std::ostream& os, Enum v)                          \
    {                                                                           \
        return os << enum_to_string(v);                                         \
    }

} // namespace reflect

namespace std {

template <::reflect::concepts::enumerable T>
struct formatter<T> : formatter<string>
{
    auto format(T value, format_context& ctx) const
    {
        std::ostringstream oss;
        oss << value;
        return formatter<string>::format(oss.str(), ctx);
    }
};

} // namespace std
