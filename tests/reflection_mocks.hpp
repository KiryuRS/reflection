// Copyright (c) 2025 KiryuRS
// SPDX-License-Identifier: MIT

#pragma once

#include "../include/reflect/enum.hpp"
#include "../include/reflect/reflect.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace tests::mocks {

// ---- enums ----

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

ENUM_PRINTABLE(some_enum, (VALUE_0, VALUE_1, VALUE_2, VALUE_3));
ENUM_PRINTABLE(another_enum, (OFFICIAL, UNOFFICIAL));

// ---- core mocks ----

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

    REFLECT(foo, (), (l, i, s, c));
};

struct bar
{
    std::string_view str_view1;
    std::string_view str_view2;
    another_enum tag;
    double price;

    REFLECT_PRINTABLE(bar, (), (str_view1, str_view2, tag, price));
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

    REFLECT_PRINTABLE(fooz, (), (fooz, ae));
};

// ---- extended mocks ----

// REFLECT_PRINTABLE generates `oss << object.member` for every field, so members must
// have operator<<. That rules out containers (vector, map, etc.) for these two structs.

// base carries entity-like metadata with a mix of string, numeric, and enum fields.
struct base
{
    std::string      name;
    uint32_t         id;
    another_enum     status;
    double           score;
    std::string_view category;

    REFLECT_PRINTABLE(base, (), (name, id, status, score, category));
};

// base_2 models lightweight scheduling / priority data.
struct base_2
{
    float    weight;
    int16_t  priority;
    bool     active;

    REFLECT_PRINTABLE(base_2, (), (weight, priority, active));
};

// derived_more inherits from both bases and adds its own members.
struct derived_more : base, base_2
{
    int              x;
    some_enum        kind;
    std::string_view note;

    REFLECT_PRINTABLE(derived_more, (base, base_2), (x, kind, note));
};

// Uses plain REFLECT (not REFLECT_PRINTABLE), so members do not need operator<< — this
// allows containers, optionals, maps, nested plain-reflected structs, and anything else.
struct with_functions
{
    // data members — deliberately heterogeneous
    int                                     x;
    double                                  y;
    std::string_view                        label;
    std::array<int, 3>                      buffer;       // fixed-size homogeneous array
    std::vector<std::string>                tags;         // heap-allocated sequence
    std::unordered_map<std::string, double> registry;     // associative container
    std::optional<another_enum>             maybe_status; // nullable enum

    // member functions — deliberately varied return types and argument lists
    int  add(int a, int b)              { return a + b; }
    double scale(double factor)         { return y * factor; }
    void reset(int new_x, double new_y) { x = new_x; y = new_y; }
    std::optional<int> find_in_buffer(int target)
    {
        for (auto v : buffer)
            if (v == target) return v;
        return std::nullopt;
    }
    void fill_buffer(std::array<int, 3> src) { buffer = src; }

    REFLECT(with_functions, (), (x, y, label, buffer, tags, registry, maybe_status,
                                 add, scale, reset, find_in_buffer, fill_buffer));
};

} // namespace tests::mocks
