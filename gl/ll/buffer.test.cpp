#include <catch2/catch.hpp>
#include "test/common.h"
#include "gl/ll/buffer.h"

TEST_CASE("Buffer Objects", "[gl][buffer]")
{
    PLATFORM_SETUP();
    using namespace GL::LL;

    SECTION("Buffer is invalid before first binding.")
    {
        auto b = Buffer(Buffer::Target::ARRAY, Buffer::Usage::DYNAMIC_DRAW);
        REQUIRE(not b);

        SECTION("Buffer is valid after first binding.")
        {
            auto bind = BufferBinding(b);
            REQUIRE(b);
        }
    }
}
