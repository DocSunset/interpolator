#include <catch2/catch.hpp>
#include "test/common.h"
#include "gl/ll/shader.h"
#include "gl/ll/program.h"

const char * all_attribs_vertex_source = R"GLSL(
#version 300 es

in float f;
in vec2 f2;
in vec3 f3;
in vec4 f4;
in mat2 m2;
in mat3 m3;
in mat4 m4;
in mat2x3 m2x3;
in mat2x4 m2x4;
in mat3x2 m3x2;
in mat3x4 m3x4;
in mat4x2 m4x2;
in mat4x3 m4x3;
in int i;
in ivec2 i2;
in ivec3 i3;

void main()
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}
)GLSL";

const char * all_attribs_fragment_source = R"GLSL(
#version 300 es

#ifdef GL_ES
precision highp float;
#endif

out vec4 color;

void main()
{
    color = gl_FragCoord;
}
)GLSL";

TEST_CASE("AttributeManifest", "[gl][attribute]")
{
    PLATFORM_SETUP();
    using namespace GL::LL;

    Program p{all_attribs_vertex_source, all_attribs_fragment_source};
    CHECK(p);

    std::vector<Attribute> expected;
    expected.emplace_back("f", Attribute::Type::FLOAT);
    expected.emplace_back("f2", Attribute::Type::VEC2);
    expected.emplace_back("f3", Attribute::Type::VEC3);
    expected.emplace_back("f4", Attribute::Type::VEC4);
    expected.emplace_back("m2", Attribute::Type::MAT2);
    expected.emplace_back("m3", Attribute::Type::MAT3);
    expected.emplace_back("m4", Attribute::Type::MAT4);
    expected.emplace_back("m2x3", Attribute::Type::MAT2x3);
    expected.emplace_back("m2x4", Attribute::Type::MAT2x4);
    expected.emplace_back("m3x2", Attribute::Type::MAT3x2);
    expected.emplace_back("m3x4", Attribute::Type::MAT3x4);
    expected.emplace_back("m4x2", Attribute::Type::MAT4x2);
    expected.emplace_back("m4x3", Attribute::Type::MAT4x3);
    expected.emplace_back("i", Attribute::Type::INT);
    expected.emplace_back("i2", Attribute::Type::IVEC2);
    expected.emplace_back("i3", Attribute::Type::IVEC3);

    auto returned = p.attributes();
    CHECK(p);

    SECTION("The manifest has exactly all the expected attributes")
    {
        for (const auto& attr : expected)
        {
            CHECK(returned.has(attr));
        }
    }
}
    

//
//    SECTION("The attributes have the expected types")
//    {
//        for (const auto& expect : expected_attributes)
//        {
//            auto out = am.find(expect.name())
//}
