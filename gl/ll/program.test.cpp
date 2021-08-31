#include <catch2/catch.hpp>
#include "test/common.h"
#include "gl/ll/program.h"
#include "gl/ll/shader.h"

TEST_CASE("Program", "[gl][program]")
{
    PLATFORM_SETUP();

    using namespace GL::LL;

    SECTION("Shaders can be attached")
    {
        Program p{};
        VertexShader v{};
        FragmentShader f{};

        CHECK(p.attach_vertex_shader(v));
        CHECK(p.attach_fragment_shader(f));
    }
}
