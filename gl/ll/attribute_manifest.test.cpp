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
in int i;
in ivec2 i2;
in ivec3 i3;

void main()
{
    float a = f + f2[0] + f3[0] + f4[0]
            + m2[0][0] + m3[0][0] + m4[0][0];
    int b = i + i2[0] + i3[0];
    gl_Position = vec4(a, float(b), 0.0, 1.0);
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

    auto returned = p.attributes();
    CHECK(p);

    SECTION("The manifest can be copied")
    {
        AttributeManifest b = p.attributes();
        b = returned;
        REQUIRE(b == returned);
    }

    SECTION("The manifest has exactly all the expected attributes")
    {
        std::vector<Attribute> expected;
        expected.emplace_back("f", Attribute::Type::FLOAT);
        expected.emplace_back("f2", Attribute::Type::VEC2);
        expected.emplace_back("f3", Attribute::Type::VEC3);
        expected.emplace_back("f4", Attribute::Type::VEC4);
        expected.emplace_back("m2", Attribute::Type::MAT2);
        expected.emplace_back("m3", Attribute::Type::MAT3);
        expected.emplace_back("m4", Attribute::Type::MAT4);
        expected.emplace_back("i", Attribute::Type::INT);
        expected.emplace_back("i2", Attribute::Type::IVEC2);
        expected.emplace_back("i3", Attribute::Type::IVEC3);
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
