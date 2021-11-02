#include <catch2/catch.hpp>
#include "test/common.h"
#include "gl/ll/texture.h"

TEST_CASE("Texture Objects", "[gl][texture]")
{
    PLATFORM_SETUP();
    using namespace GL::LL;

    SECTION("Texture is invalid before first binding.")
    {
        auto t = Texture();
        CHECK(not t);
    }
}
