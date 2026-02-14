#pragma once

#include "concepts.hpp"
#include "convertors.hpp"
#include "../reflect/reflect.hpp"

#include <format>
#include <utility>

namespace argparse {

namespace detail {

struct arg_supplied_info
{
    std::string_view name;
    bool supplied;
};

void validate_args(const auto& args_supplied)
{
    std::string parse_error_msg;
    const char* delimiter = "";

    for (const auto& arg_supplied_info : args_supplied)
    {
        if (arg_supplied_info.supplied)
            continue;

        parse_error_msg += std::exchange(delimiter, ", ");
        parse_error_msg += arg_supplied_info.name;
    }

    if (!parse_error_msg.empty())
    {
        throw std::invalid_argument(std::format("[argparse] arguments not supplied (or no default values): {}", parse_error_msg));
    }
}

template <typename MemberType>
bool has_default_value(MemberType& member_variable)
{
    if constexpr (concepts::same_as_optional<MemberType>)
    {
        return member_variable.has_value();
    }
    // acceptable for containers to be empty as default value
    else if constexpr (concepts::same_as_vector<MemberType>)
    {
        return true;
    }
    return false;
}

} // namespace detail

template <::reflect::concepts::reflectable T>
constexpr T parse_args(int argc, const char* argv[])
{
    using namespace std::string_view_literals;

    /*
     * Rules (and usage) for parse_args:
     * 1. Only supports prefix "--" followed by the variable name. e.g. --count
     * 2. Value after the key is denoted by a whitespace. Invalid syntax results in the entire argument ignored
     * 3. With the above, this means to parse an argument, requires minimally 2 arguments. e.g. --count 42
     * 4. Any type conversion issues will result in a runtime error
     * 5. For boolean types, only expect "true" or "false" (case insensitive)
     * 6. For list (vectors), separate them by comma
     */

    if (argc <= 1)
    {
        throw std::invalid_argument("[argparse] no arguments supplied!");
    }

    const std::vector<std::string_view> vectorized_args{argv, argv + argc};

    // TODO: use inplace_vector from C++26
    std::array<detail::arg_supplied_info, T::meta_info_array().size()> args_supplied;

    T parsed{};

    ::reflect::for_each<T>([&parsed, &vectorized_args, &args_supplied, i=0] <typename Descriptor>() mutable {
        using member_type = Descriptor::member_type;
        auto& member_variable = ::reflect::get_member_variable<Descriptor>(parsed);

        detail::arg_supplied_info info{Descriptor::name, detail::has_default_value(member_variable)};

        auto iter = std::ranges::find_if(vectorized_args, [] (std::string_view arg) {
            // TODO: Support for short form? (e.g. -c for --config)
            return arg.starts_with("--") && arg.substr(2) == Descriptor::name;
        });

        // conditional check here ensures that the corresponding value is legitimate
        if (iter != vectorized_args.end() && iter + 1 != vectorized_args.end() && !(iter + 1)->starts_with("--"))
        {
            const std::string_view value = *(iter + 1);

            if (const auto result = convertors::parse_value<member_type>(value); result.has_value())
            {
                member_variable = result.value();
                info.supplied = true;
            }
            else
            {
                throw std::invalid_argument(std::format("[argparse] failed to parse {} of type {}. Please check arguments!", Descriptor::name, Descriptor::mem_type_str));
            }
        }
        args_supplied[i++] = std::move(info);
    });

    detail::validate_args(args_supplied);
    return parsed;
}

} // namespace argparse
