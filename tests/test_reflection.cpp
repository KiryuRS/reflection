#include "../include/reflect/enum.hpp"
#include "../include/reflect/reflect.hpp"

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

    REFLECT(foo, (), (l, i, s, c)); // used when we just want the bare reflection
};

struct bar
{
    std::string_view str_view1;
    std::string_view str_view2;
    another_enum tag;
    double price;

    REFLECT_PRINTABLE(bar, (), (str_view1, str_view2, tag, price)); // used when we want reflection + printable + other encapsulated logic
};

struct baz
{
    bar b;
    float f;

    REFLECT_PRINTABLE(baz, (), (f, b));
};

struct fooz
{
    int fooz;
    another_enum ae;

    REFLECT_PRINTABLE(fooz, (), (fooz, ae)); // even with member variable named as the class, reflection still works
};

struct goo
{
    long l;
    int i;
    std::string_view view;
};

struct base
{
    std::string str;
    double d;

    REFLECT_PRINTABLE(base, (), (str, d));
};

struct base_2
{
    char c;

    REFLECT_PRINTABLE(base_2, (), (c));
};

struct derived_more : base, base_2
{
    int x;

    REFLECT_PRINTABLE(derived_more, (base, base_2), (x));
};

struct with_functions
{
    int x;
    void some_foo_function(int, double, const derived_more&);

    REFLECT(with_functions, (), (x, some_foo_function));
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

TEST(test_reflection, test_derived)
{
    mocks::derived_more dm;
    dm.x = 100;
    dm.c = 'A';
    dm.str = "Hello World";
    dm.d = 3.14;

    std::cout << std::format("{}\n", dm);
    std::cout << print_meta(dm) << '\n';
}

TEST(test_reflection, test_with_functions)
{
    mocks::with_functions obj{.x = 42};

    bool has_member_function = false;
    ::reflect::for_each<mocks::with_functions>([&] <typename Descriptor>() mutable {
        if constexpr (std::is_function_v<typename Descriptor::member_type>)
        {
            has_member_function = true;
            auto fn = ::reflect::get_member_variable<Descriptor>(obj);
            static_assert(std::is_invocable_v<decltype(fn), int, double, const mocks::derived_more&>);
        }
        else
        {
            EXPECT_EQ(::reflect::get_member_variable<Descriptor>(obj), 42);
        }
    });

    EXPECT_TRUE(has_member_function);
}

TEST(test_reflection, test_descriptor_for)
{
    // descriptor_for<T, &T::member> resolves to the generated descriptor type for that member

    // basic member variable lookup
    using desc_l = reflect::descriptor_for<mocks::foo, &mocks::foo::l>;
    using desc_i = reflect::descriptor_for<mocks::foo, &mocks::foo::i>;
    using desc_s = reflect::descriptor_for<mocks::foo, &mocks::foo::s>;
    using desc_c = reflect::descriptor_for<mocks::foo, &mocks::foo::c>;

    static_assert(desc_l::name == "l");
    static_assert(desc_i::name == "i");
    static_assert(desc_s::name == "s");
    static_assert(desc_c::name == "c");

    // class_type must be the owning struct
    static_assert(std::same_as<desc_l::class_type, mocks::foo>);
    static_assert(std::same_as<desc_i::class_type, mocks::foo>);

    // member_type must match the actual C++ type of the field
    static_assert(std::same_as<desc_l::member_type, long>);
    static_assert(std::same_as<desc_i::member_type, int>);
    static_assert(std::same_as<desc_s::member_type, short>);
    static_assert(std::same_as<desc_c::member_type, char>);

    // member_pointer_type must match decltype of the pointer
    static_assert(std::same_as<desc_l::member_pointer_type, decltype(&mocks::foo::l)>);
    static_assert(std::same_as<desc_i::member_pointer_type, decltype(&mocks::foo::i)>);

    // lookup works across different reflectable structs
    using desc_price = reflect::descriptor_for<mocks::bar, &mocks::bar::price>;
    static_assert(desc_price::name == "price");
    static_assert(std::same_as<desc_price::class_type, mocks::bar>);
    static_assert(std::same_as<desc_price::member_type, double>);

    using desc_tag = reflect::descriptor_for<mocks::bar, &mocks::bar::tag>;
    static_assert(desc_tag::name == "tag");
    static_assert(std::same_as<desc_tag::member_type, mocks::another_enum>);

    // lookup works for own members of a derived struct
    using desc_x = reflect::descriptor_for<mocks::derived_more, &mocks::derived_more::x>;
    static_assert(desc_x::name == "x");
    static_assert(std::same_as<desc_x::class_type, mocks::derived_more>);
    static_assert(std::same_as<desc_x::member_type, int>);

    // lookup works for member functions
    using desc_fn = reflect::descriptor_for<mocks::with_functions, &mocks::with_functions::some_foo_function>;
    static_assert(desc_fn::name == "some_foo_function");
    static_assert(std::same_as<desc_fn::class_type, mocks::with_functions>);
    static_assert(std::is_function_v<desc_fn::member_type>);

    // the resolved descriptor satisfies the descriptor_like concept
    static_assert(reflect::concepts::descriptor_like<desc_l>);
    static_assert(reflect::concepts::descriptor_like<desc_price>);
    static_assert(reflect::concepts::descriptor_like<desc_fn>);

    // mem_ptr round-trips back to the original pointer
    static_assert(desc_l::mem_ptr == &mocks::foo::l);
    static_assert(desc_price::mem_ptr == &mocks::bar::price);
    static_assert(desc_x::mem_ptr == &mocks::derived_more::x);
}

} // namespace tests
