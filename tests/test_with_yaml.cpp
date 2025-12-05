#include "../include/reflect.hpp"

#include <yaml-cpp/yaml.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <concepts>
#include <ranges>
#include <sstream>

namespace tests {

using namespace ::testing;

namespace concepts {

namespace detail {

template <typename T>
struct is_optional : std::false_type
{
};

template <typename... Args>
struct is_optional<std::optional<Args...>> : std::true_type
{
};

} // namespace detail

template <typename T, typename RawT = std::remove_cvref_t<T>>
concept container_like = requires(RawT)
{
    // though std::string is something like a container, but we don't really regard it as one
    // hence we should ensure that its not a std::string or std::string_view
    requires !std::same_as<std::string, RawT>;
    requires !std::same_as<std::string_view, RawT>;
    typename RawT::value_type;
    typename RawT::allocator_type;
    requires std::ranges::range<RawT>;
};

template <typename T>
concept same_as_optional = detail::is_optional<std::remove_cvref_t<T>>::value;

} // namespace concepts

} // namespace tests


// setup for serializing yaml string to a type with reflection
namespace YAML {

template <::reflect::concepts::reflectable T>
struct convert<T>
{
    static Node encode(const T& obj)
    {
        Node node{};
        ::reflect::for_each<T>([&node, &obj] <typename Descriptor> () {
            using member_type = typename Descriptor::member_type;
            const auto& member = ::reflect::get_member_variable<Descriptor>(obj);

            if constexpr (tests::concepts::container_like<member_type>)
            {
                if (member.empty())
                    return;
                node[Descriptor::name] = member;
            }
            else if constexpr (tests::concepts::same_as_optional<member_type>)
            {
                if (!member)
                    return;
                node[Descriptor::name] = *member;
            }
            else
            {
                node[Descriptor::name] = member;
            }
        });
        return node;
    }

    static bool decode(const Node& node, T& obj)
    {
        // possible for pre-validation here. but for the sake of testing, this section is omitted

        ::reflect::for_each<T>([&node, &obj] <typename Descriptor> () {
            using member_type = typename Descriptor::member_type;
            auto& member = ::reflect::get_member_variable<Descriptor>(obj);

            if constexpr (tests::concepts::container_like<member_type>)
            {
                if (!node[Descriptor::name])
                    return;
                member = node[Descriptor::name].template as<member_type>();
            }
            else if constexpr (tests::concepts::same_as_optional<member_type>)
            {
                if (!node[Descriptor::name])
                    return;
                member = node[Descriptor::name].template as<typename member_type::value_type>();
            }
            else
            {
                member = node[Descriptor::name].template as<member_type>();
            }
        });
        return true;
    }

};

} // namespace YAML


namespace tests {

template <reflect::concepts::reflectable T>
T yaml_deserialize(const std::string& yaml_config_str)
{
    constexpr std::string_view type_name = ::reflect::utility::get_short_name<T>();
    const YAML::Node node = YAML::Load(yaml_config_str);
    return node[type_name].as<T>();
}

template <reflect::concepts::reflectable T>
std::string yaml_serialize(const T& obj)
{
    constexpr std::string_view type_name = ::reflect::utility::get_short_name<T>();
    YAML::Node node;
    node[type_name] = obj;
    std::ostringstream oss;
    oss << node;
    return oss.str();
}


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

    const auto converted = yaml_deserialize<mocks::built_in_types>(yaml_str);
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

    const auto converted = yaml_deserialize<mocks::complex_types>(yaml_str);
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
    const auto converted = yaml_deserialize<mocks::built_in_types>(built_in_types_str);
    const std::string yaml_str = yaml_serialize(converted);
    const auto round_trip_converted = yaml_deserialize<mocks::built_in_types>(yaml_str);
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

    const auto converted = yaml_deserialize<mocks::complex_types>(complex_types_str);
    const std::string yaml_str = yaml_serialize(converted);
    const auto round_trip_converted = yaml_deserialize<mocks::complex_types>(yaml_str);
    EXPECT_EQ(converted, round_trip_converted);
}

} // namespace tests
