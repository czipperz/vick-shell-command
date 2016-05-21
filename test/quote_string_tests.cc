/* Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/ */

#include "catch.hpp"

namespace vick {
namespace shell_command {
std::string quote_string_windows(const std::string&);
std::string quote_string_linux(const std::string&);
}
}

using vick::shell_command::quote_string_windows;
using vick::shell_command::quote_string_linux;

TEST_CASE("quote_string_windows") {
    REQUIRE(quote_string_windows("hi man") == "\"hi man\"");
    REQUIRE(quote_string_windows("hi \" man") == "\"hi \\\" man\"");
    REQUIRE(quote_string_windows("") == "\"\"");
}

TEST_CASE("quote_string_linux") {
    REQUIRE(quote_string_linux("hi man") == "'hi man'");
    REQUIRE(quote_string_linux("hi ' man") == "'hi '\\'' man'");
    REQUIRE(quote_string_linux("") == "''");
}
