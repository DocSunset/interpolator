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

    SECTION("Attaching a new shader automatically calls glDetachShader to make way")
    {
        Program p{};
        VertexShader v1{};
        VertexShader v2{};

        p.attach_vertex_shader(v1);
        REQUIRE(p.attach_vertex_shader(v2));
    }

    SECTION("Trying to attach the same shader twice skips the repeated error-generating call to glAttachShader.")
    {
        Program p{};
        VertexShader v{};
        p.attach_vertex_shader(v);
        REQUIRE(p.attach_vertex_shader(v));
    }

    SECTION("Program links successfully given compiled shaders.")
    {
    }
}
