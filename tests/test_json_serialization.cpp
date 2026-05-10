// Copyright (c) 2025 KiryuRS
// SPDX-License-Identifier: MIT

#include "../include/json/parser.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace tests {

using namespace ::testing;

namespace mocks {

struct json_primitives
{
    bool            active;
    char            letter;
    int32_t         count;
    int64_t         big_count;
    uint64_t        unsigned_big;
    float           rate;
    double          precision;
    const char*     cstr;
    std::string_view sv;
    std::string     name;

    REFLECT(json_primitives, (), (active, letter, count, big_count, unsigned_big, rate, precision, cstr, sv, name));
};

struct json_compound
{
    json_primitives                          inner;
    std::vector<int>                         numbers;
    std::vector<std::string>                 tags;
    std::unordered_map<std::string, int>     registry;
    std::optional<int>                       maybe_int;
    std::optional<std::string>               maybe_str;

    REFLECT(json_compound, (), (inner, numbers, tags, registry, maybe_int, maybe_str));
};

} // namespace mocks

TEST(test_json_serialization, serialize_primitive_and_string_types)
{
    const mocks::json_primitives obj{
        .active       = true,
        .letter       = 'Z',
        .count        = -100,
        .big_count    = -9'000'000'000LL,
        .unsigned_big = 18'000'000'000ULL,
        .rate         = 1.5f,
        .precision    = 3.14,
        .cstr         = "c-string",
        .sv           = "a-view",
        .name         = "alice",
    };
    const std::string result = json::serialize(obj);
    EXPECT_THAT(result, StartsWith(R"({"json_primitives": )"));
    EXPECT_THAT(result, HasSubstr(R"("active": true)"));
    EXPECT_THAT(result, HasSubstr(R"("letter": "Z")"));
    EXPECT_THAT(result, HasSubstr(R"("count": -100)"));
    EXPECT_THAT(result, HasSubstr(R"("big_count": -9000000000)"));
    EXPECT_THAT(result, HasSubstr(R"("unsigned_big": 18000000000)"));
    EXPECT_THAT(result, HasSubstr(R"("rate": 1.5)"));
    EXPECT_THAT(result, HasSubstr(R"("precision": 3.14)"));
    EXPECT_THAT(result, HasSubstr(R"("cstr": "c-string")"));
    EXPECT_THAT(result, HasSubstr(R"("sv": "a-view")"));
    EXPECT_THAT(result, HasSubstr(R"("name": "alice")"));
}

TEST(test_json_serialization, serialize_complex_types)
{
    // fully populated — nested struct, all container types, both optionals present
    const mocks::json_compound full{
        .inner        = {true, 'A', 10, 1000LL, 9999ULL, 1.5f, 1.5, "nested", "view", "world"},
        .numbers      = {1, 2, 3},
        .tags         = {"foo", "bar"},
        .registry     = {{"alpha", 1}, {"beta", 2}},
        .maybe_int    = 7,
        .maybe_str    = "present",
    };
    const std::string full_result = json::convert_to_json(full);
    EXPECT_THAT(full_result, HasSubstr(R"("numbers": [1, 2, 3])"));
    EXPECT_THAT(full_result, HasSubstr(R"("tags": ["foo", "bar"])"));
    EXPECT_THAT(full_result, HasSubstr(R"("maybe_int": 7)"));
    EXPECT_THAT(full_result, HasSubstr(R"("maybe_str": "present")"));
    // unordered_map order is non-deterministic — assert each entry individually
    EXPECT_THAT(full_result, HasSubstr(R"("alpha": 1)"));
    EXPECT_THAT(full_result, HasSubstr(R"("beta": 2)"));

    // also print it out to verify against existing json formatters online
    std::cout << full_result << '\n';

    // sparse — both optionals absent, sequence containers empty
    const mocks::json_compound sparse{
        .inner        = {false, 'B', 0, 0LL, 0ULL, 0.0f, 0.0, "", "", ""},
        .numbers      = {},
        .tags         = {},
        .registry     = {},
        .maybe_int    = std::nullopt,
        .maybe_str    = std::nullopt,
    };
    const std::string sparse_result = json::convert_to_json(sparse);
    EXPECT_THAT(sparse_result, Not(HasSubstr("maybe_int")));
    EXPECT_THAT(sparse_result, Not(HasSubstr("maybe_str")));
    EXPECT_THAT(sparse_result, HasSubstr(R"("numbers": [])"));
    EXPECT_THAT(sparse_result, HasSubstr(R"("tags": [])"));
}

} // namespace tests
