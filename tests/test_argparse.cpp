#include "../include/argparse/argparse.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <source_location>
#include <stdexcept>
#include <tuple>

namespace tests {

using namespace ::testing;

namespace mocks {

struct arg_options
{
    std::optional<float> pi = 3.14159f;
    int count;
    std::string backup_type;
    bool enabled;
    std::vector<int> values;

    REFLECT(arg_options, (pi, count, backup_type, enabled, values))
};

struct arg_optional_options
{
    int dummy;
    std::optional<std::string> backup_directory;
    std::optional<int> interval = 100;

    REFLECT(arg_optional_options, (dummy, backup_directory, interval))
};

} // namespace mocks

template <typename T, std::convertible_to<std::exception> ExceptionT = std::invalid_argument>
void expect_exception(int argc,
                      const char* (&argv)[],
                      std::string_view error_message,
                      std::source_location current_loc = std::source_location::current())
{
    const std::string failed_loc = std::format("Failed at {}:{}", current_loc.file_name(), current_loc.line());

    bool exception_thrown = false;
    try
    {
        std::ignore = ::argparse::parse_args<T>(argc, argv);
    }
    catch (const ExceptionT& e)
    {
        exception_thrown = true;
        EXPECT_EQ(std::string{e.what()}, error_message) << failed_loc;
    }

    EXPECT_TRUE(exception_thrown) << failed_loc;
}

TEST(test_argparse, test_empty_args_should_throw)
{
    const char* argv[] = {"my_exe"};
    const int argc = std::ranges::size(argv);
    expect_exception<mocks::arg_options>(argc, argv, "[argparse] no arguments supplied!");
}

TEST(test_argparse, test_invalid_arg_should_throw)
{
    const char* argv[] = {"my_exe",
                          "--count", "c"};
    const int argc = std::ranges::size(argv);
    expect_exception<mocks::arg_options>(argc, argv, "[argparse] failed to parse count of type int. Please check arguments!");
}

TEST(test_argparse, test_failed_conversion_for_list_should_throw)
{
    const char* argv[] = {"my_exe",
                          "--count", "10",
                          "--backup_type", "NONE",
                          "--enabled", "true",
                          "--values", "1,hi,3"};
    const int argc = std::ranges::size(argv);
    expect_exception<mocks::arg_options>(argc, argv, "[argparse] failed to parse values of type std::vector<int>. Please check arguments!");
}

TEST(test_argparse, test_default_values)
{
    const char* argv[] = {"another_executable",
                          "--count", "999",
                          "--backup_type", "LOCAL",
                          "--enabled", "TRUE"};
    const int argc = std::ranges::size(argv);
    const auto argparsed = ::argparse::parse_args<mocks::arg_options>(argc, argv);

    EXPECT_DOUBLE_EQ(argparsed.pi.value(), 3.14159f);
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

    EXPECT_DOUBLE_EQ(argparsed.pi.value(), 6.19f);
    EXPECT_EQ(argparsed.count, 12345);
    EXPECT_EQ(argparsed.backup_type, "CLOUD");
    EXPECT_EQ(argparsed.enabled, false);
    EXPECT_THAT(argparsed.values, ElementsAre(10, 11, 12, 13, 14, 15));
}

TEST(test_argparse, test_optional_all_supplied)
{
    const char* argv[] = {"dummy", "--dummy", "1", "--backup_directory", "/apps/user/tmp", "--interval", "3600"};
    const int argc = std::ranges::size(argv);
    const auto argparsed = ::argparse::parse_args<mocks::arg_optional_options>(argc, argv);

    EXPECT_EQ(argparsed.dummy, 1);
    EXPECT_EQ(argparsed.backup_directory.value(), "/apps/user/tmp");
    EXPECT_EQ(argparsed.interval.value(), 3600);
}

TEST(test_argparse, test_optional_no_default_value)
{
    const char* argv[] = {"dummy", "--dummy", "0"};
    const int argc = std::ranges::size(argv);
    expect_exception<mocks::arg_optional_options>(argc, argv, "[argparse] arguments not supplied (or no default values): backup_directory");
}

} // namespace tests
