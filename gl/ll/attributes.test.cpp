#include <catch2/catch.hpp>
#include "test/common.h"
#include "gl/ll/shader.h"
#include "gl/ll/program.h"
#include "gl/ll/attributes.h"
#include "test/shader_sources.h"

const char * vertex_shader = R"GLSL(
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
in ivec4 i4;
in uint u;
in uvec2 u2;
in uvec3 u3;
in uvec4 u4;

void main()
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}
)GLSL";

const char * fragment_shader = R"GLSL(
#version 300 es

#ifdef GL_ES
precision highp float;
#endif

in vec4 color_in;
out vec4 color;

void main()
{
    color = color_in;
}
)GLSL";

TEST_CASE("Attributes" "[gl][attribute]")
{
    PLATFORM_SETUP();

    // make an attribute manifest from the program
    using namespace GL::LL;

    SECTION("Given an attribute constructed by name and type")
    {
        std::string name = "test";
        auto type = Attribute::Type::FLOAT;
        Attribute a{name.c_str(), type};

        SECTION("name matches name")
        {
            REQUIRE(std::string(a.name()) == name);
        }

        SECTION("type matches type")
        {
            REQUIRE(a.type() == type);
        }
    }

    SECTION("Attributes with the same name and type compare equal")
    {
        Attribute a{"foo", Attribute::Type::FLOAT};
        Attribute b{"foo", Attribute::Type::FLOAT};

        REQUIRE(a == b);
    }

    SECTION("Attributes can be copied")
    {
        Attribute orig{"foo", Attribute::Type::FLOAT};

        SECTION("By constructor")
        {
            Attribute copy{orig};
            REQUIRE(copy == orig);
        }

        SECTION("By assignment")
        {
            Attribute copy{"bar", Attribute::Type::INT};
            copy = orig;
            REQUIRE(copy == orig);
        }
    }

    SECTION("Attributes can be moved")
    {
        std::string name = "foo";
        auto type = Attribute::Type::FLOAT;
        auto make_attrib = [&](){return Attribute(name.c_str(), type);};

        SECTION("By constructor")
        {
            Attribute move{make_attrib()};
            REQUIRE(std::string(move.name()) == name);
            REQUIRE(move.type() == type);
        }

        SECTION("By assignment")
        {
            Attribute move{"bar", Attribute::Type::INT};
            move = make_attrib();
            REQUIRE(std::string(move.name()) == name);
            REQUIRE(move.type() == type);
        }
    }

    SECTION("Attributes can be constructed from programs")
    {
        Program p{vertex_source, fragment_source};
        CHECK(p);

        auto expected = Attribute("pos", Attribute::Type::VEC2);
        auto returned = p.attributes();
        CHECK(std::string(returned.name()) == std::string("pos"));
        CHECK(Attribute::Type::VEC2 == returned.type());
        REQUIRE(returned == expected);
    }
}

//    std::vector<Attribute> expected_attributes;
//    expected_attributes.emplace_back("f", GL_FLOAT, 1);
//    expected_attributes.emplace_back("f2", GL_FLOAT, 2);
//    expected_attributes.emplace_back("f3", GL_FLOAT, 3);
//    expected_attributes.emplace_back("f4", GL_FLOAT, 4);
//    expected_attributes.emplace_back("m2", GL_FLOAT, 2);
//    expected_attributes.emplace_back("m3", GL_FLOAT, 1);
//    expected_attributes.emplace_back("m4", GL_FLOAT, 1);
//    expected_attributes.emplace_back("m2x3", GL_FLOAT, 1);
//    expected_attributes.emplace_back("m2x4", GL_FLOAT, 1);
//    expected_attributes.emplace_back("m3x2", GL_FLOAT, 1);
//    expected_attributes.emplace_back("m3x4", GL_FLOAT, 1);
//    expected_attributes.emplace_back("m4x2", GL_FLOAT, 1);
//    expected_attributes.emplace_back("m4x3", GL_FLOAT, 1);
//    expected_attributes.emplace_back("i", GL_FLOAT, 1);
//    expected_attributes.emplace_back("i2", GL_FLOAT, 1);
//    expected_attributes.emplace_back("i3", GL_FLOAT, 1);
//    expected_attributes.emplace_back("i4", GL_FLOAT, 1);
//    expected_attributes.emplace_back("u", GL_FLOAT, 1);
//    expected_attributes.emplace_back("u2", GL_FLOAT, 1);
//    expected_attributes.emplace_back("u3", GL_FLOAT, 1);
//    expected_attributes.emplace_back("u4", GL_FLOAT, 1);
//}
//    SECTION("The manifest has exactly all the expected attributes")
//    {
//        for (const auto& attr : expected_attributes)
//        {
//            CHECK(am.has(attr));
//        }
//    }
//
//    SECTION("The attributes have the expected types")
//    {
//        for (const auto& expect : expected_attributes)
//        {
//            auto out = am.find(expect.name())
//}
