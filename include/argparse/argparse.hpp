#pragma once

#include "concepts.hpp"
#include "convertors.hpp"
#include "../reflect/reflect.hpp"

#include <format>
#include <span>

namespace argparse {

namespace detail {

inline std::span<std::string_view> cmdline_to_span(int argc, const char* argv[])
{
    static std::vector<std::string_view> args{argv, argv + argc};
    return args;
}

} // namespace detail

template <::reflect::concepts::reflectable T>
constexpr T parse_args(int argc, const char* argv[])
{
    /*
     * Rules (and usage) for parse_args:
     * 1. Only supports prefix "--" followed by the variable name. e.g. --count
     * 2. Value after the key is denoted by a whitespace. Invalid syntax results in the entire argument ignored
     * 3. With the above, this means to parse an argument, requires minimally 2 arguments. e.g. --count 42
     * 4. Any type conversion issues will result in a runtime error
     * 5. For boolean types, only expect "true" or "false"
     * 6. For list (vectors), separate them by comma
     */

    if (argc < 1)
    {
        return T{};
    }

    const std::vector<std::string_view> vectorized_args{argv, argv + argc};

    T parsed{};

    ::reflect::for_each<T>([&parsed, &vectorized_args] <typename Descriptor>() {
        auto iter = std::ranges::find_if(vectorized_args, [] (std::string_view arg) {
            // TODO: Support for short form? (e.g. -c for --config)
            return arg.starts_with("--") && arg.substr(2) == Descriptor::name;
        });

        // conditional check here ensures that the corresponding value is legitimate
        if (iter != vectorized_args.end() && iter + 1 != vectorized_args.end() && !(iter + 1)->starts_with("--"))
        {
            using member_type = Descriptor::member_type;
            const std::string_view value = *(iter + 1);
            auto& member_variable = ::reflect::get_member_variable<Descriptor>(parsed);

            if (const auto result = convertors::parse_value<member_type>(value); result.has_value())
            {
                member_variable = result.value();
            }
            else
            {
                // for non optional type, throw exception
                if constexpr (!concepts::same_as_optional<T>)
                {
                    throw std::invalid_argument(std::format("[argparse] failed to parse {} of type {}. Please check arguments!", Descriptor::name, Descriptor::mem_type_str));
                }
                else if (!member_variable.has_value())
                {
                    throw std::invalid_argument(std::format("[argparse] {} is of type {}, but no default value and no args passed!", Descriptor::name, Descriptor::mem_type_str));
                }
            }
        }
    });

    return parsed;
}

} // namespace argparse
