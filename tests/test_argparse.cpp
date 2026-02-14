#include "../include/argparse/argparse.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace tests {

using namespace ::testing;

namespace mocks {

struct arg_options
{
    float pi = 3.14159f;
    int count;
    std::string backup_type;
    bool enabled;
    std::vector<int> values;

    REFLECT(arg_options, (pi, count, backup_type, enabled, values))
};

struct arg_optional_options
{
    std::optional<std::string> backup_directory;
    std::optional<int> interval = 100;

    REFLECT(arg_optional_options, (backup_directory, interval))
};

} // namespace mocks

TEST(test_argparse, test_invalid_arg_should_throw)
{
    const char* argv[] = {"my_exe",
                          "--count", "c"};
    const int argc = std::ranges::size(argv);
    bool exception_thrown = false;
    try
    {
        std::ignore = ::argparse::parse_args<mocks::arg_options>(argc, argv);
    }
    catch (const std::invalid_argument& e)
    {
        exception_thrown = true;
        EXPECT_STREQ(e.what(), "[argparse] failed to parse count of type int. Please check arguments!");
    }

    EXPECT_TRUE(exception_thrown);
}

TEST(test_argparse, test_failed_conversion_for_list_should_throw)
{
    const char* argv[] = {"my_exe",
                          "--count", "10",
                          "--backup_type", "NONE",
                          "--enabled", "true",
                          "--values", "1,hi,3"};
    const int argc = std::ranges::size(argv);
    bool exception_thrown = false;
    try
    {
        std::ignore = ::argparse::parse_args<mocks::arg_options>(argc, argv);
    }
    catch (const std::invalid_argument& e)
    {
        exception_thrown = true;
        EXPECT_STREQ(e.what(), "[argparse] failed to parse values of type std::vector<int>. Please check arguments!");
    }

    EXPECT_TRUE(exception_thrown);
}

TEST(test_argparse, test_default_values)
{
    const char* argv[] = {"another_executable",
                          "--count", "999",
                          "--backup_type", "LOCAL",
                          "--enabled", "TRUE"};
    const int argc = std::ranges::size(argv);
    const auto argparsed = ::argparse::parse_args<mocks::arg_options>(argc, argv);

    EXPECT_DOUBLE_EQ(argparsed.pi, 3.14159f);
    EXPECT_EQ(argparsed.count, 999);
    EXPECT_EQ(argparsed.backup_type, "LOCAL");
    EXPECT_EQ(argparsed.enabled, true);
    EXPECT_THAT(argparsed.values, ElementsAre());
}

TEST(test_argparse, test_complete_arguments)
{
    const char* argv[] = {"the_executable",
                          "--pi", "6.19",
                          "--count", "12345",
                          "--backup_type", "CLOUD",
                          "--enabled", "FaLsE", // should be case insensitive
                          "--values", "10,11,12,13,14,15"};
    const int argc = std::ranges::size(argv);
    const auto argparsed = ::argparse::parse_args<mocks::arg_options>(argc, argv);

    EXPECT_DOUBLE_EQ(argparsed.pi, 6.19f);
    EXPECT_EQ(argparsed.count, 12345);
    EXPECT_EQ(argparsed.backup_type, "CLOUD");
    EXPECT_EQ(argparsed.enabled, false);
    EXPECT_THAT(argparsed.values, ElementsAre(10, 11, 12, 13, 14, 15));
}

} // namespace tests
