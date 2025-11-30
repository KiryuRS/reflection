#pragma once

#include <source_location>
#include <string_view>

namespace reflect::utility {

template <typename T>
consteval const char* get_function_name()
{
    return std::source_location::current().function_name();
}

template <typename T>
consteval std::string_view get_name()
{
    constexpr std::string_view void_function_name = get_function_name<void>();
    constexpr size_t start_index = void_function_name.find("void");
    static_assert(start_index != std::string_view::npos, "Possible that compiler changed how std::source_location works. Please revisit me!");

    constexpr std::string_view t_function_name = get_function_name<T>();
    constexpr size_t end_index = t_function_name.find(']', start_index);
    static_assert(end_index != std::string_view::npos, "Possible that compiler changed how std::source_location works. Please revisit me!");

    return t_function_name.substr(start_index, end_index - start_index);
}

// this removes the namespaces
// e.g. std::vector<int> -> vector<int>
//      some::long::namespace::vector<std::string> -> vector<std::string>
//      foo<some_class::deep::within>::type -> type
template <typename T>
consteval std::string_view get_short_name()
{
    constexpr std::string_view full_type = get_name<T>();
    size_t levels = 0, pos = std::string_view::npos;
    for (size_t i = 0; i != full_type.length(); ++i)
    {
        char c = full_type[i];
        if (c == '<')
            ++levels;
        else if (c == '>')
            --levels;

        if (levels == 0 && c == ':')
            pos = i;
    }
    return pos == std::string_view::npos ? full_type : full_type.substr(pos + 1);
}

} // namespace reflect::utility
