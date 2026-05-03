// Copyright (c) 2025 KiryuRS
// SPDX-License-Identifier: MIT

#include "reflection_mocks.hpp"

#include <gtest/gtest.h>

namespace tests {

TEST(test_reflection_core, test_struct_traits_and_concepts)
{
    // standard layout must be preserved
    static_assert(std::is_standard_layout_v<mocks::foo_no_reflect>);
    static_assert(std::is_standard_layout_v<mocks::foo>);

    // aggregate must be preserved
    static_assert(std::is_aggregate_v<mocks::foo_no_reflect>);
    static_assert(std::is_aggregate_v<mocks::foo>);

    // trivial construction / copy / move must be preserved
    static_assert(std::is_trivially_default_constructible_v<mocks::foo_no_reflect>);
    static_assert(std::is_trivially_default_constructible_v<mocks::foo>);
    static_assert(std::is_trivially_copy_constructible_v<mocks::foo_no_reflect>);
    static_assert(std::is_trivially_copy_constructible_v<mocks::foo>);
    static_assert(std::is_trivially_move_constructible_v<mocks::foo_no_reflect>);
    static_assert(std::is_trivially_move_constructible_v<mocks::foo>);

    // sizeof must not change
    static_assert(sizeof(mocks::foo) == sizeof(mocks::foo_no_reflect));
    static_assert(sizeof(mocks::bar) == sizeof(mocks::bar_no_reflect));
    static_assert(sizeof(mocks::baz) == sizeof(mocks::baz_no_reflect));

    // concepts::reflectable passes only for structs with REFLECT / REFLECT_PRINTABLE
    static_assert(!reflect::concepts::reflectable<mocks::foo_no_reflect>);
    static_assert(reflect::concepts::reflectable<mocks::foo>);
    static_assert(reflect::concepts::reflectable<mocks::bar>);

    // concepts::reflect_and_printable requires REFLECT_PRINTABLE — REFLECT alone is not enough
    static_assert(!reflect::concepts::reflect_and_printable<mocks::foo>);
    static_assert(reflect::concepts::reflect_and_printable<mocks::bar>);
    static_assert(reflect::concepts::reflect_and_printable<mocks::baz>);
}

TEST(test_reflection_core, test_for_each)
{
    // iteration: walk every member and confirm the count
    constexpr mocks::foo f1{.l = 100, .i = 99, .s = 98, .c = 'A'};
    std::size_t member_count = 0;
    reflect::for_each<mocks::foo>([&] <typename Descriptor>() { ++member_count; });
    EXPECT_EQ(member_count, 4u);

    // roundtrip: assign each member of f2 from f1 via for_each, then verify equality
    mocks::foo f2{};
    reflect::for_each<mocks::foo>([&] <typename Descriptor>() {
        reflect::get_member_variable<Descriptor>(f2) = reflect::get_member_variable<Descriptor>(f1);
    });
    reflect::for_each<mocks::foo>([&] <typename Descriptor>() {
        EXPECT_EQ(reflect::get_member_variable<Descriptor>(f1), reflect::get_member_variable<Descriptor>(f2));
    });

    // enum roundtrip: enum_to_string -> string_to_enum must recover the original value
    constexpr auto e0 = mocks::some_enum::VALUE_1;
    EXPECT_EQ(e0, string_to_enum(mocks::some_enum{}, enum_to_string(e0)));

    constexpr auto e1 = mocks::another_enum::UNOFFICIAL;
    EXPECT_EQ(e1, string_to_enum(mocks::another_enum{}, enum_to_string(e1)));
}

TEST(test_reflection_core, test_printable)
{
    constexpr mocks::bar b1{
        .str_view1 = "Hello World",
        .str_view2 = "long string",
        .tag       = mocks::another_enum::OFFICIAL,
        .price     = 69.0,
    };
    constexpr mocks::bar b2 = b1;
    constexpr mocks::baz bb1{.b = b1, .f = 3.14f};
    constexpr mocks::baz bb2 = bb1;
    constexpr mocks::fooz fz{.fooz = 100, .ae = mocks::another_enum::OFFICIAL};

    // to_string (ADL): equal structs must produce identical strings
    EXPECT_EQ(to_string(b1), to_string(b2));
    EXPECT_EQ(to_string(bb1), to_string(bb2));

    // operator<< via std::cout — including nested struct and enum values
    std::cout << b1 << '\n';
    std::cout << bb1 << '\n';
    std::cout << fz << '\n';
    std::cout << mocks::another_enum::UNOFFICIAL << '\n';

    // std::format — including nested struct and enum values
    std::cout << std::format("{}\n", b1);
    std::cout << std::format("{}\n", bb1);
    std::cout << std::format("{}\n", fz);
    std::cout << std::format("{}\n", mocks::some_enum::VALUE_2);
}

} // namespace tests
