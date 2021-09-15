#include <catch2/catch.hpp>
#include "test/common.h"
#include "gl/ll/shader.h"
#include "gl/ll/program.h"
#include "gl/ll/attribute.h"
#include "gl/ll/attribute_manifest.h"

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

    auto returned = AttributeManifest(p);

    SECTION("The manifest can be copied")
    {
        SECTION("By constructor")
        {
            AttributeManifest b{returned};
            REQUIRE(b == returned);
        }
        SECTION("By operator")
        {
            AttributeManifest b{};
            b = returned;
            REQUIRE(b == returned);
        }
    }

    SECTION("The manifest can be moved")
    {
        GLint size = returned.size();
        auto a = returned[0];
        AttributeManifest b = std::move(returned);
        CHECK(b.size() == size);
        CHECK(a == b[0]);
    }

    SECTION("The manifest has exactly all the expected attributes")
    {
        std::vector<Attribute> expected;
        expected.emplace_back("f", AttributeType::FLOAT);
        expected.emplace_back("f2", AttributeType::VEC2);
        expected.emplace_back("f3", AttributeType::VEC3);
        expected.emplace_back("f4", AttributeType::VEC4);
        expected.emplace_back("m2", AttributeType::MAT2);
        expected.emplace_back("m3", AttributeType::MAT3);
        expected.emplace_back("m4", AttributeType::MAT4);
        expected.emplace_back("i", AttributeType::INT);
        expected.emplace_back("i2", AttributeType::IVEC2);
        expected.emplace_back("i3", AttributeType::IVEC3);

        CHECK(returned.size() == expected.size());

        for (const auto& attr : expected)
        {
            CHECK(returned.has(attr));
        }
    }

    SECTION("Manifest can be constructed manually")
    {
        auto expected = AttributeManifest();
        expected.add_attribute("f", AttributeType::FLOAT);
        expected.add_attribute("f2", AttributeType::VEC2);
        expected.add_attribute("f3", AttributeType::VEC3);
        expected.add_attribute("f4", AttributeType::VEC4);
        expected.add_attribute("m2", AttributeType::MAT2);
        expected.add_attribute("m3", AttributeType::MAT3);
        expected.add_attribute("m4", AttributeType::MAT4);
        expected.add_attribute("i", AttributeType::INT);
        expected.add_attribute("i2", AttributeType::IVEC2);
        expected.add_attribute("i3", AttributeType::IVEC3);

        REQUIRE(expected == returned);
    }

    SECTION("Attributes can be retrieved by name")
    {
        auto a = Attribute("f", AttributeType::FLOAT);
        auto b = returned["f"];

        REQUIRE(a == b);
    }

    SECTION("Number of AttributeElements to store can be retrieved")
    {
        std::size_t expected = (1 + 2 + 3 + 4 + 4 + 9 + 16)
                             + (1 + 2 + 3);
        REQUIRE(returned.elements() == expected);
    }

    SECTION("Element offset of attributes can be retrieved")
    {
        SECTION("by name")
        {
            CHECK(returned.offset_of("f") == 0);
            REQUIRE(returned.offset_of("f2") == returned["f"].elements());
        }
        SECTION("by index")
        {
            CHECK(returned.offset_of(int(0)) == 0);
            REQUIRE(returned.offset_of(1) == returned[0].elements());
        }
    }

    SECTION("Index of attribute can be retrieved by name")
    {
        CHECK(returned.index_of("f") == 0);
        CHECK(returned.index_of("f2") == 1);
        CHECK(returned.index_of("f3") == 2);
        CHECK(returned.index_of("i3") == 9);
    }
}
