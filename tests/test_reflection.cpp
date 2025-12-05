#include "../include/enum.hpp"
#include "../include/reflect.hpp"

#include <gtest/gtest.h>

namespace tests {

namespace mocks {

enum class some_enum : uint16_t
{
    NONE = 0,
    VALUE_0,
    VALUE_1,
    VALUE_2,
    VALUE_3,
};

enum class another_enum : uint16_t
{
    NONE = 0,
    OFFICIAL,
    UNOFFICIAL,
};

ENUM_PRINTABLE(mocks::some_enum, (VALUE_0, VALUE_1, VALUE_2, VALUE_3));
ENUM_PRINTABLE(mocks::another_enum, (OFFICIAL, UNOFFICIAL));

// enums must be "reflectable" first

struct foo_no_reflect
{
    long l;
    int i;
    short s;
    char c;
};

struct bar_no_reflect
{
    std::string_view str_view1;
    std::string_view str_view2;
    another_enum tag;
    double price;
};

struct baz_no_reflect
{
    bar_no_reflect b;
    float f;
};

struct foo
{
    long l;
    int i;
    short s;
    char c;

    REFLECT(foo, (l, i, s, c)); // used when we just want the bare reflection
};

struct bar
{
    std::string_view str_view1;
    std::string_view str_view2;
    another_enum tag;
    double price;

    REFLECT_PRINTABLE(bar, (str_view1, str_view2, tag, price)); // used when we want reflection + printable + other encapsulated logic
};

struct baz
{
    bar b;
    float f;

    REFLECT_PRINTABLE(baz, (f, b));
};

struct fooz
{
    int fooz;
    another_enum ae;

    REFLECT_PRINTABLE(fooz, (fooz, ae)); // even with member variable named as the class, reflection still works
};

struct goo
{
    long l;
    int i;
    std::string_view view;
};

} // namespace mocks

TEST(test_enum, test_roundtrip)
{
    const auto e0 = mocks::some_enum::VALUE_1;
    const auto value0 = enum_to_string(e0);
    EXPECT_EQ(e0, string_to_enum(mocks::some_enum{}, value0));

    std::cout << std::format("{}\n", e0);

    const auto e1 = mocks::another_enum::UNOFFICIAL;
    const auto value1 = enum_to_string(e1);
    EXPECT_EQ(e1, string_to_enum(mocks::another_enum{}, value1));

    std::cout << e1 << '\n';
}

TEST(test_reflection, test_class_traits_should_remain_same)
{
    // layout specifications
    static_assert(std::is_standard_layout_v<mocks::foo_no_reflect>);
    static_assert(std::is_standard_layout_v<mocks::foo>);
    static_assert(std::is_aggregate_v<mocks::foo_no_reflect>);
    static_assert(std::is_aggregate_v<mocks::foo>);

    // construction
    static_assert(std::is_trivially_default_constructible_v<mocks::foo_no_reflect>);
    static_assert(std::is_trivially_default_constructible_v<mocks::foo>);
    static_assert(std::is_trivially_copy_constructible_v<mocks::foo_no_reflect>);
    static_assert(std::is_trivially_copy_constructible_v<mocks::foo>);
    static_assert(std::is_trivially_move_constructible_v<mocks::foo_no_reflect>);
    static_assert(std::is_trivially_move_constructible_v<mocks::foo>);
    static_assert(std::is_trivially_default_constructible_v<mocks::foo_no_reflect>);
    static_assert(std::is_trivially_default_constructible_v<mocks::foo>);

    // sizeof classes should be the same
    static_assert(sizeof(mocks::foo) == sizeof(mocks::foo_no_reflect));
    static_assert(sizeof(mocks::bar) == sizeof(mocks::bar_no_reflect));
    static_assert(sizeof(mocks::baz) == sizeof(mocks::baz_no_reflect));
}

TEST(test_reflection, test_generate_meta_info)
{
    // should be able to use aggregate initialization and constexpr
    constexpr mocks::foo f1{.l = 100, .i = 99, .s = 98, .c = 'A'};
    constexpr mocks::foo f2 = f1;

    reflect::for_each<mocks::foo>([&f1, &f2] <typename Descriptor> () {
        EXPECT_EQ(reflect::get_member_variable<Descriptor>(f1), reflect::get_member_variable<Descriptor>(f2));
    });
}

TEST(test_reflection, test_reflect)
{
    // should be possible to use aggregate initialization and constexpr
    constexpr mocks::bar b1{.str_view1 = "Hello World", .str_view2 = "Some static very long long long long string", .tag = mocks::another_enum::OFFICIAL, .price = 69.0};
    constexpr mocks::bar b2 = b1;

    EXPECT_EQ(to_string(b1), to_string(b2));

    constexpr mocks::baz bb1{.b = b1, .f = 3.14f};
    constexpr mocks::baz bb2{.b = b1, .f = 3.14f};

    EXPECT_EQ(to_string(bb1), to_string(bb2));

    constexpr mocks::fooz fz1{.fooz = 100, .ae = mocks::another_enum::OFFICIAL};

    std::cout << fz1 << '\n';

    std::cout << print_meta(b1) << '\n';

    // reflect::for_each<mocks::goo>([] (auto) {}); // should not compile because there's no meta generated for goo!
}

TEST(test_reflection, test_formatter)
{
    [[maybe_unused]] constexpr mocks::foo f1{.l = 100, .i = 99, .s = 98, .c = 'A'};
    constexpr mocks::bar b1{.str_view1 = "Hello World", .str_view2 = "Some static very long long long long string", .tag = mocks::another_enum::OFFICIAL, .price = 69.0};
    constexpr mocks::baz bb1{.b = b1, .f = 3.14f};

    std::cout << std::format("{}\n", b1);
    std::cout << std::format("{}\n", bb1);
    // std::cout << std::format("{}\n", f1); // should not compile because to_string is not defined!
}

} // namespace tests
