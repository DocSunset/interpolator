#include <catch2/catch.hpp>
#include "test/common.h"
#include "gl/ll/shader.h"
#include "gl/ll/attributes.h"
#include "gl/ll/attribute_manifest.h"
#include "gl/ll/program.h"
#include "test/shader_sources.h"

TEST_CASE("Attributes" "[gl][attribute]")
{
    PLATFORM_SETUP();

    // make an attribute manifest from the program
    using namespace GL::LL;

    SECTION("Given an attribute constructed by name and type")
    {
        std::string name = "test";
        auto type = AttributeType::FLOAT;
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
        Attribute a{"foo", AttributeType::FLOAT};
        Attribute b{"foo", AttributeType::FLOAT};

        REQUIRE(a == b);
    }

    SECTION("Attributes can be copied")
    {
        Attribute orig{"foo", AttributeType::FLOAT};

        SECTION("By constructor")
        {
            Attribute copy{orig};
            REQUIRE(copy == orig);
        }

        SECTION("By assignment")
        {
            Attribute copy{"bar", AttributeType::INT};
            copy = orig;
            REQUIRE(copy == orig);
        }
    }

    SECTION("Attributes can be moved")
    {
        std::string name = "foo";
        auto type = AttributeType::FLOAT;
        auto make_attrib = [&](){return Attribute(name.c_str(), type);};

        SECTION("By constructor")
        {
            Attribute move{make_attrib()};
            REQUIRE(std::string(move.name()) == name);
            REQUIRE(move.type() == type);
        }

        SECTION("By assignment")
        {
            Attribute move{"bar", AttributeType::INT};
            move = make_attrib();
            REQUIRE(std::string(move.name()) == name);
            REQUIRE(move.type() == type);
        }
    }

    SECTION("Attributes can be constructed from programs")
    {
        Program p{vertex_source, fragment_source};
        CHECK(p);

        auto expected = Attribute("pos", AttributeType::VEC2);
        auto attributes = AttributeManifest(p);
        auto returned = attributes[0];
        CHECK(std::string(returned.name()) == std::string("pos"));
        CHECK(AttributeType::VEC2 == returned.type());
        REQUIRE(returned == expected);
    }
}
