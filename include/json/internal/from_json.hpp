// Copyright (c) 2025 KiryuRS
// SPDX-License-Identifier: MIT

#pragma once

#include <sstream>

namespace json::internal {

inline void skip_whitespace(std::istringstream& iss)
{
    iss >> std::ws;
}

inline void expect(std::istringstream& iss, char expected)
{
    skip_whitespace(iss);

    const int c = iss.peek();
    if (c == std::char_traits<char>::eof())
    {
        throw std::runtime_error{"unexpected end of input. Expected: '" + std::string{expected} + "'"};
    }

    if (static_cast<char>(c) != expected)
    {
        throw std::runtime_error{"expected: '" + std::string{expected} + "', got: '" + static_cast<char>(c) + "'"};
    }

    iss.ignore();
}

inline std::string parse_string(std::istringstream& iss)
{
    // e.g. "key"
    skip_whitespace(iss);
    // TODO: Implement me!
    return {};
}

} // namespace json::internal
