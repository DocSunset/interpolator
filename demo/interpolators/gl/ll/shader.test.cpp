#include <string>
#include <catch2/catch.hpp>
#include "test/common.h"
#include "gl/ll/shader.h"

static const GLchar * source = R"GLSL(
#version 300 es

in vec2 pos;
out vec2 position;

void main()
{
    gl_Position = vec4(pos, 0.0, 1.0);
    position = vec2(pos[0], pos[1]);
}
)GLSL";

static const GLchar * bad_source = R"GLSL(
#version 300 es

in vec2 pos
out vec2 position;

void ()
{
    gl_Position = vec4(pos, 0.0, 1.0)
    position  vec2(ps[0], pos[1]);

)GLSL";

TEST_CASE("Shader", "[gl][shader]")
{
    PLATFORM_SETUP();

    using namespace GL::LL;

    SECTION("Shader handle constructor")
    {
        Shader sh{Shader::Type::Vertex};
        REQUIRE(sh);
    }

    SECTION("Deleting the shader handle invalidates the shader")
    {
        Shader sh{Shader::Type::Vertex};
        sh.delete_handle();
        REQUIRE(not sh);
    }

    SECTION("Set source / get source match")
    {
        Shader sh{Shader::Type::Vertex};
        sh.set_source(source);
        GLchar * source_ = sh.source();
        REQUIRE(std::string(source) == std::string(source_));
        free(source_);
    }

    SECTION("Compile succeeds")
    {
        Shader sh{Shader::Type::Vertex};
        sh.set_source(source);
        sh.compile();
        REQUIRE(sh.compile_status() == true);
    }

    SECTION("Bad compile fails")
    {
        Shader sh{Shader::Type::Vertex};
        sh.set_source(bad_source);
        sh.compile();
        REQUIRE(sh.compile_status() == false);
        sh.print_info_log();
    }
}
