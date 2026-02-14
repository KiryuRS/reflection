#include "../include/yaml/parser.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <concepts>
#include <ranges>
#include <sstream>

namespace tests {

using namespace ::testing;

namespace mocks {

struct built_in_types
{
    float floating_value;
    int integer_value;
    char char_value;
    double double_value;
    long long_value;

    // for unit-testing
    auto operator<=>(const built_in_types&) const = default;

    REFLECT(built_in_types, (floating_value, integer_value, char_value, double_value, long_value));
};

struct simple
{
    int value_1;

    // for unit-testing
    auto operator<=>(const simple&) const = default;

    REFLECT(simple, (value_1));
};

struct complex_types
{
    std::vector<int> vector;
    std::string string;
    std::unordered_map<int, int> unordered_map;
    std::optional<double> optional;
    std::vector<double> empty_vector;
    std::optional<int> empty_optional;
    simple s;

    // for unit-testing
    auto operator<=>(const complex_types&) const = default;

    REFLECT(complex_types, (vector, string, unordered_map, optional, empty_vector, empty_optional, s));
};

} // namespace mocks

TEST(test_yaml_with_reflection, test_serializable_built_in_type_wrapped)
{
    const std::string yaml_str = R"(
built_in_types:
    floating_value: 111.2
    integer_value: 20
    char_value: 'A'
    double_value: 3.14
    long_value: 99712
)";

    const auto converted = ::yaml::deserialize<mocks::built_in_types>(yaml_str);
    constexpr mocks::built_in_types expected{.floating_value = 111.2f, .integer_value = 20, .char_value = 'A', .double_value = 3.14, .long_value = 99712};
    EXPECT_EQ(converted, expected);
}

TEST(test_yaml_with_reflection, test_serializable_complex_type)
{
    const std::string yaml_str = R"(
complex_types:
    vector: [1,2,3,4,5]
    s:
        value_1: 10
    unordered_map:
        1: 10
        2: 20
    string: "Hello World"
    optional: 69.69
)";

    const auto converted = ::yaml::deserialize<mocks::complex_types>(yaml_str);
    EXPECT_THAT(converted.vector, ElementsAre(1, 2, 3, 4, 5));
    EXPECT_THAT(converted.unordered_map, UnorderedElementsAre(Pair(1, 10), Pair(2, 20)));
    EXPECT_EQ(converted.optional, std::optional<double>(69.69));
    EXPECT_EQ(converted.string, "Hello World");
    EXPECT_EQ(converted.s.value_1, 10);

    EXPECT_TRUE(converted.empty_vector.empty());
    EXPECT_FALSE(converted.empty_optional.has_value());

    // cannot be constexpr because of vector and string
    const mocks::complex_types expected{
        .vector = {1, 2, 3, 4, 5},
        .string = "Hello World",
        .unordered_map = {std::make_pair(1, 10), std::make_pair(2, 20)},
        .optional = 69.69,
        .empty_vector = {},
        .empty_optional = {},
        .s = 10
    };
    EXPECT_EQ(converted, expected);
}

TEST(test_yaml_with_reflection, test_round_trip_built_in_types)
{
    const std::string built_in_types_str = R"(
built_in_types:
    floating_value: 202376.1
    integer_value: 64644
    char_value: 'Z'
    double_value: 770766.112
    long_value: 62660662
)";
    const auto converted = ::yaml::deserialize<mocks::built_in_types>(built_in_types_str);
    const std::string yaml_str = ::yaml::serialize(converted);
    const auto round_trip_converted = ::yaml::deserialize<mocks::built_in_types>(yaml_str);
    EXPECT_EQ(converted, round_trip_converted);
}

TEST(test_yaml_with_reflection, test_round_trip_complex_types)
{
    const std::string complex_types_str = R"(
complex_types:
    vector: [1]
    s:
        value_1: 777
    unordered_map:
        50: 166
    string: "Goodbye World"
    optional: 11.11
    empty_optional: 2
)";

    const auto converted = ::yaml::deserialize<mocks::complex_types>(complex_types_str);
    const std::string yaml_str = ::yaml::serialize(converted);
    const auto round_trip_converted = ::yaml::deserialize<mocks::complex_types>(yaml_str);
    EXPECT_EQ(converted, round_trip_converted);
}

} // namespace tests
