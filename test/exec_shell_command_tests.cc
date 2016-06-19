#include "catch.hpp"
#include <ncurses.h>

#include "../lib.hh"
#include "../src/esc.hh"
#include "operating_system_macros.hh"

using namespace vick;
using namespace vick::shell_command;

#if IS_OS_LINUX || IS_OS_MAC
TEST_CASE("Output of echo into contents", "[shell_command]") {
    contents c;
    {
        // The version that takes a contents& displays it periodically
        // to the screen.  This ensures that doesn't cause segfaults
        // and shuts down correctly.
        visual_setup _;
        exec_shell_command("echo hi", c);
    }
    CHECK(c.cont.size() == 1);
    CHECK(c.cont[0] == "hi");
}

TEST_CASE("Output of echo into string", "[shell_command]") {
    // everything stuffed into a string
    std::string str;
    exec_shell_command("echo hi", &str);
    CHECK(str == "hi\n");

    str.clear();
    exec_shell_command("echo hi >&2", &str);
    CHECK(str == "hi\n");

    str.clear();
    exec_shell_command("echo hi; echo bye >&2", &str);
    CHECK(str == "hi\nbye\n");
}

TEST_CASE("Output of echo into two strings", "[shell_command]") {
    // seperated streams
    std::string out, err;
    exec_shell_command("echo hi; echo bye >&2", &out, &err);
    CHECK(out == "hi\n");
    CHECK(err == "bye\n");
}
#endif
