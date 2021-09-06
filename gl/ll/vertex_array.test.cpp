#include <catch2/catch.hpp>
#include "test/common.h"
#include "gl/ll/vertex_array.h"

TEST_CASE("Vertex Array Objects", "[gl][vao]")
{
    PLATFORM_SETUP();
    using namespace GL::LL;

    SECTION("VertexArray is invalid before first binding.")
    {
        auto vao = VertexArray();
        REQUIRE(not vao);

        SECTION("VertexArray is valid after first binding.")
        {
            auto binding = bind(vao);
            REQUIRE(vao);
        }
    }

}
