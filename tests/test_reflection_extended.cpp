// Copyright (c) 2025 KiryuRS
// SPDX-License-Identifier: MIT

#include "reflection_mocks.hpp"

#include <gtest/gtest.h>

namespace tests {

TEST(test_reflection_extended, test_function_descriptor)
{
    mocks::with_functions obj{
        .x            = 10,
        .y            = 2.5,
        .label        = "reflect",
        .buffer       = {1, 2, 3},
        .tags         = {"alpha", "beta", "gamma"},
        .registry     = {{"pi", 3.14}, {"e", 2.71}},
        .maybe_status = mocks::another_enum::OFFICIAL,
    };

    std::size_t variable_count = 0;
    std::size_t function_count = 0;

    reflect::for_each<mocks::with_functions>([&] <typename D>() mutable {
        if constexpr (std::is_function_v<typename D::member_type>)
        {
            ++function_count;

            using args = typename D::introspection_type::arguments_type;
            using ret  = typename D::introspection_type::return_type;

            if constexpr (D::name == "add")
            {
                // add(int, int) -> int
                static_assert(std::same_as<ret, int>);
                static_assert(reflect::typelist_size_v<args> == 2);
                static_assert(std::same_as<reflect::typelist_element_t<0, args>, int>);
                static_assert(std::same_as<reflect::typelist_element_t<1, args>, int>);

                auto fn = reflect::get_member_variable<D>(obj);
                EXPECT_EQ(fn(3, 4), 7);
                EXPECT_EQ(fn(-1, 1), 0);
            }
            else if constexpr (D::name == "scale")
            {
                // scale(double) -> double
                static_assert(std::same_as<ret, double>);
                static_assert(reflect::typelist_size_v<args> == 1);
                static_assert(std::same_as<reflect::typelist_element_t<0, args>, double>);

                auto fn = reflect::get_member_variable<D>(obj);
                EXPECT_DOUBLE_EQ(fn(2.0), 5.0);  // obj.y (2.5) * 2.0
                EXPECT_DOUBLE_EQ(fn(0.0), 0.0);
            }
            else if constexpr (D::name == "reset")
            {
                // reset(int, double) -> void
                static_assert(std::same_as<ret, void>);
                static_assert(reflect::typelist_size_v<args> == 2);
                static_assert(std::same_as<reflect::typelist_element_t<0, args>, int>);
                static_assert(std::same_as<reflect::typelist_element_t<1, args>, double>);

                auto fn = reflect::get_member_variable<D>(obj);
                fn(99, 0.1);  // mutates obj — verified after the loop
            }
            else if constexpr (D::name == "find_in_buffer")
            {
                // find_in_buffer(int) -> std::optional<int>
                static_assert(std::same_as<ret, std::optional<int>>);
                static_assert(reflect::typelist_size_v<args> == 1);
                static_assert(std::same_as<reflect::typelist_element_t<0, args>, int>);

                auto fn = reflect::get_member_variable<D>(obj);
                EXPECT_EQ(fn(2), std::optional<int>{2});
                EXPECT_EQ(fn(99), std::nullopt);
            }
            else if constexpr (D::name == "fill_buffer")
            {
                // fill_buffer(std::array<int, 3>) -> void
                static_assert(std::same_as<ret, void>);
                static_assert(reflect::typelist_size_v<args> == 1);
                static_assert(std::same_as<reflect::typelist_element_t<0, args>, std::array<int, 3>>);

                auto fn = reflect::get_member_variable<D>(obj);
                fn(std::array<int, 3>{7, 8, 9});  // mutates obj.buffer — verified after the loop
            }
        }
        else
        {
            ++variable_count;

            if constexpr (D::name == "x")
            {
                static_assert(std::same_as<typename D::member_type, int>);
                EXPECT_EQ(reflect::get_member_variable<D>(obj), 10);
            }
            else if constexpr (D::name == "y")
            {
                static_assert(std::same_as<typename D::member_type, double>);
                EXPECT_DOUBLE_EQ(reflect::get_member_variable<D>(obj), 2.5);
            }
            else if constexpr (D::name == "label")
            {
                static_assert(std::same_as<typename D::member_type, std::string_view>);
                EXPECT_EQ(reflect::get_member_variable<D>(obj), "reflect");
            }
            else if constexpr (D::name == "buffer")
            {
                static_assert(std::same_as<typename D::member_type, std::array<int, 3>>);
                const auto& buf = reflect::get_member_variable<D>(obj);
                EXPECT_EQ(buf[0], 1);
                EXPECT_EQ(buf[1], 2);
                EXPECT_EQ(buf[2], 3);
            }
            else if constexpr (D::name == "tags")
            {
                static_assert(std::same_as<typename D::member_type, std::vector<std::string>>);
                const auto& t = reflect::get_member_variable<D>(obj);
                EXPECT_EQ(t.size(), 3u);
                EXPECT_EQ(t[0], "alpha");
            }
            else if constexpr (D::name == "registry")
            {
                static_assert((std::same_as<typename D::member_type,
                                            std::unordered_map<std::string, double>>));
                const auto& reg = reflect::get_member_variable<D>(obj);
                EXPECT_EQ(reg.size(), 2u);
                EXPECT_DOUBLE_EQ(reg.at("pi"), 3.14);
            }
            else if constexpr (D::name == "maybe_status")
            {
                static_assert(std::same_as<typename D::member_type, std::optional<mocks::another_enum>>);
                const auto& ms = reflect::get_member_variable<D>(obj);
                EXPECT_TRUE(ms.has_value());
                EXPECT_EQ(*ms, mocks::another_enum::OFFICIAL);
            }
        }
    });

    EXPECT_EQ(variable_count, 7u);
    EXPECT_EQ(function_count, 5u);

    // reset(99, 0.1) was called through reflection — verify the mutation landed
    EXPECT_EQ(obj.x, 99);
    EXPECT_DOUBLE_EQ(obj.y, 0.1);

    // fill_buffer({7, 8, 9}) was called through reflection — verify the buffer changed
    EXPECT_EQ(obj.buffer[0], 7);
    EXPECT_EQ(obj.buffer[1], 8);
    EXPECT_EQ(obj.buffer[2], 9);
}

TEST(test_reflection_extended, test_descriptor_for)
{
    // name, class_type, member_type, member_pointer_type round-trip for foo
    using desc_l = reflect::descriptor_for<mocks::foo, &mocks::foo::l>;
    using desc_i = reflect::descriptor_for<mocks::foo, &mocks::foo::i>;
    using desc_s = reflect::descriptor_for<mocks::foo, &mocks::foo::s>;
    using desc_c = reflect::descriptor_for<mocks::foo, &mocks::foo::c>;

    static_assert(desc_l::name == "l");
    static_assert(desc_i::name == "i");
    static_assert(desc_s::name == "s");
    static_assert(desc_c::name == "c");

    static_assert(std::same_as<desc_l::class_type, mocks::foo>);
    static_assert(std::same_as<desc_i::class_type, mocks::foo>);

    static_assert(std::same_as<desc_l::member_type, long>);
    static_assert(std::same_as<desc_i::member_type, int>);
    static_assert(std::same_as<desc_s::member_type, short>);
    static_assert(std::same_as<desc_c::member_type, char>);

    static_assert(std::same_as<desc_l::member_pointer_type, decltype(&mocks::foo::l)>);
    static_assert(std::same_as<desc_i::member_pointer_type, decltype(&mocks::foo::i)>);

    // mem_ptr must round-trip back to the original pointer
    static_assert(desc_l::mem_ptr == &mocks::foo::l);
    static_assert(desc_i::mem_ptr == &mocks::foo::i);

    // lookup works across different reflectable structs
    using desc_price = reflect::descriptor_for<mocks::bar, &mocks::bar::price>;
    static_assert(desc_price::name == "price");
    static_assert(std::same_as<desc_price::class_type, mocks::bar>);
    static_assert(std::same_as<desc_price::member_type, double>);
    static_assert(desc_price::mem_ptr == &mocks::bar::price);

    using desc_tag = reflect::descriptor_for<mocks::bar, &mocks::bar::tag>;
    static_assert(desc_tag::name == "tag");
    static_assert(std::same_as<desc_tag::member_type, mocks::another_enum>);

    // descriptor_like concept must be satisfied
    static_assert(reflect::concepts::descriptor_like<desc_l>);
    static_assert(reflect::concepts::descriptor_like<desc_price>);

    // lookup works for complex-typed data members
    using desc_tags     = reflect::descriptor_for<mocks::with_functions, &mocks::with_functions::tags>;
    using desc_registry = reflect::descriptor_for<mocks::with_functions, &mocks::with_functions::registry>;
    using desc_maybe    = reflect::descriptor_for<mocks::with_functions, &mocks::with_functions::maybe_status>;

    static_assert(desc_tags::name == "tags");
    static_assert(std::same_as<desc_tags::member_type, std::vector<std::string>>);

    static_assert(desc_registry::name == "registry");
    static_assert((std::same_as<desc_registry::member_type, std::unordered_map<std::string, double>>));

    static_assert(desc_maybe::name == "maybe_status");
    static_assert(std::same_as<desc_maybe::member_type, std::optional<mocks::another_enum>>);

    // lookup works for member functions
    using desc_add = reflect::descriptor_for<mocks::with_functions, &mocks::with_functions::add>;
    static_assert(desc_add::name == "add");
    static_assert(std::same_as<desc_add::class_type, mocks::with_functions>);
    static_assert(std::is_function_v<desc_add::member_type>);
    static_assert(reflect::concepts::descriptor_like<desc_add>);

    using desc_find = reflect::descriptor_for<mocks::with_functions, &mocks::with_functions::find_in_buffer>;
    static_assert(desc_find::name == "find_in_buffer");
    static_assert(std::is_function_v<desc_find::member_type>);
}

TEST(test_reflection_extended, test_derived)
{
    // base has 5 members: name, id, status, score, category
    std::size_t base_count = 0;
    reflect::for_each<mocks::base>([&] <typename Descriptor>() { ++base_count; });
    EXPECT_EQ(base_count, 5u);

    // base_2 has 3 members: weight, priority, active
    std::size_t base_2_count = 0;
    reflect::for_each<mocks::base_2>([&] <typename Descriptor>() { ++base_2_count; });
    EXPECT_EQ(base_2_count, 3u);

    // derived_more inherits all base members then adds its own (x, kind, note)
    mocks::derived_more dm;
    dm.name     = "entity-1";
    dm.id       = 42;
    dm.status   = mocks::another_enum::OFFICIAL;
    dm.score    = 9.81;
    dm.category = "physics";
    dm.weight   = 1.5f;
    dm.priority = 10;
    dm.active   = true;
    dm.x        = 100;
    dm.kind     = mocks::some_enum::VALUE_3;
    dm.note     = "multi-base derived struct";

    // for_each walks: 5 (base) + 3 (base_2) + 3 (derived_more own) = 11
    std::size_t total_count = 0;
    reflect::for_each<mocks::derived_more>([&] <typename Descriptor>() { ++total_count; });
    EXPECT_EQ(total_count, 11u);

    // printable output covers the full inheritance chain
    const std::string dm_str = to_string(dm);
    EXPECT_FALSE(dm_str.empty());
    std::cout << std::format("{}\n", dm);

    // descriptor_for resolves own members of a derived struct
    using desc_x    = reflect::descriptor_for<mocks::derived_more, &mocks::derived_more::x>;
    using desc_kind = reflect::descriptor_for<mocks::derived_more, &mocks::derived_more::kind>;
    using desc_note = reflect::descriptor_for<mocks::derived_more, &mocks::derived_more::note>;

    static_assert(desc_x::name == "x");
    static_assert(std::same_as<desc_x::class_type, mocks::derived_more>);
    static_assert(std::same_as<desc_x::member_type, int>);
    static_assert(desc_x::mem_ptr == &mocks::derived_more::x);

    static_assert(desc_kind::name == "kind");
    static_assert(std::same_as<desc_kind::member_type, mocks::some_enum>);

    static_assert(desc_note::name == "note");
    static_assert(std::same_as<desc_note::member_type, std::string_view>);
}

} // namespace tests
