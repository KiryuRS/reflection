#include "../include/reflect.hpp"

#include <gtest/gtest.h>

namespace tests {

namespace mocks {

struct foo
{
    long l;
    int i;
    short s;
    char c;

    GENERATE_META_INFO(foo, (l, i, s, c));
};

struct bar
{
    std::string_view str_view1;
    std::string_view str_view2;
    double price;

    REFLECT(bar, (str_view1, str_view2, price));
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

TEST(test_reflection, test_generate_meta_info)
{
    static_assert(sizeof(mocks::foo) == 16);

    // should be able to use aggregate initialization and constexpr
    constexpr mocks::foo f1{.l = 100, .i = 99, .s = 98, .c = 'A'};
    constexpr mocks::foo f2 = f1;

    reflect::for_each<mocks::foo>([&f1, &f2] <typename Descriptor> () {
        EXPECT_EQ(reflect::get_member_variable<Descriptor>(f1), reflect::get_member_variable<Descriptor>(f2));
    });
}

TEST(test_reflection, test_reflect)
{
    static constexpr size_t sizeof_bar = sizeof(std::string_view) + sizeof(std::string_view) + sizeof(double); // should not have any padding
    static_assert(sizeof(mocks::bar) == sizeof_bar);

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
