#include "../include/reflect.hpp"

#include <gtest/gtest.h>

namespace tests {

namespace mocks {

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

    GENERATE_META_INFO(foo, (l, i, s, c)); // used when we just want the bare reflection
};

struct bar
{
    std::string_view str_view1;
    std::string_view str_view2;
    double price;

    REFLECT(bar, (str_view1, str_view2, price)); // used when we want reflection + printable + other encapsulated logic
};

struct baz
{
    bar b;
    float f;

    REFLECT(baz, (f, b));
};

struct goo
{
    long l;
    int i;
    std::string_view view;
};

} // namespace mocks

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
    constexpr mocks::bar b1{.str_view1 = "Hello World", .str_view2 = "Some static very long long long long string", .price = 69.0};
    constexpr mocks::bar b2 = b1;

    EXPECT_EQ(to_string(b1), to_string(b2));

    constexpr mocks::baz bb1{.b = b1, .f = 3.14f};
    constexpr mocks::baz bb2{.b = b1, .f = 3.14f};

    EXPECT_EQ(to_string(bb1), to_string(bb2));
}

TEST(test_reflection, test_formatter)
{
    [[maybe_unused]] constexpr mocks::foo f1{.l = 100, .i = 99, .s = 98, .c = 'A'};
    constexpr mocks::bar b1{.str_view1 = "Hello World", .str_view2 = "Some static very long long long long string", .price = 69.0};
    constexpr mocks::baz bb1{.b = b1, .f = 3.14f};

    std::cout << std::format("{}\n", b1);
    std::cout << std::format("{}\n", bb1);
    // std::cout << std::format("{}\n", f1); // should not compile because to_string is not defined!
}

TEST(test_reflection, test_not_in_scope)
{
    // reflect::for_each<mocks::goo>([] (auto) {}); // should not compile because there's no meta generated for goo!
}

} // namespace tests
