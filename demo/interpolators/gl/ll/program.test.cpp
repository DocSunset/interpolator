#include <catch2/catch.hpp>
#include "test/common.h"
#include "gl/ll/program.h"

TEST_CASE("Program", "[gl][program]")
{
    PLATFORM_SETUP();

    using namespace GL::LL;

    SECTION("Program constructor")
    {
        Program prog{};
    }
}
