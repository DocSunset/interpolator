#include <catch2/catch.hpp>
#include "test/common.h"
#include "test/shader_sources.h"
#include "gl/ll/program.h"
#include "gl/vertex_array.h"

TEST_CASE("High-level Vertex Array", "[gl][vertex array][high-level]")
{
    PLATFORM_SETUP();

    using namespace GL;

    LL::Program p{vertex_source, fragment_source};
    VertexArray v{p};
    CHECK(v.size() == 0);
    CHECK(v.capacity() >= 0);

    SECTION("Get a template with fields for each vertex attribute")
    {
        auto form = v.add_vertex();

        CHECK(form.has("pos"));
        CHECK(form.has("bi"));

        SECTION("Attributes in the form have expected size")
        {
            auto pos = form["pos"];
            REQUIRE(pos.size() == 2);
        }

        SECTION("Derive vertices from the form")
        {
            GLfloat val1 = 1.0;
            GLfloat val2 = 2.0;
            GLfloat val3 = 3.0;
            form["pos"].set(val1, val2)
                 ["bi"].set(val3);
            CHECK(form["pos"][0]->f == val1);
            CHECK(form["pos"][1]->f == val2);
            CHECK(form["bi"][0]->f == val3);
        }
    }

    //SECTION("Attribute by name is congruent with attribute size")
    //{
    //    auto form = v.add_vertex();
    //    CHECK(form.raw() == v.raw());
    //    CHECK(form["pos"][0] == v.raw());
    //    CHECK(form["pos"][1] == v.raw() + 1);
    //    CHECK(form["bi"][0] == v.raw() + form["pos"].size());

    //    auto form2 = v.add_vertex();
    //    CHECK(form2.raw() == v.raw() + v.stride());
    //    CHECK(form2["pos"][0] == v.raw() + v.stride());
    //    CHECK(form2["pos"][1] == v.raw() + v.stride() + 1);
    //    CHECK(form2["bi"][0] == v.raw() + v.stride() + form2["pos"].size());

    //}

    //SECTION("Append data to the array by modifying forms")
    //{
    //    GLfloat val1 = 1.0;
    //    GLfloat val2 = 2.0;
    //    GLfloat val3 = 3.0;
    //    GLfloat val4 = 4.0;
    //    GLfloat val5 = 5.0;
    //    GLfloat val6 = 6.0;

    //    v.add_vertex()["pos"].set(val1, val2)
    //                   ["bi"].set(val3);
    //    v.add_vertex()["pos"].set(val4, val5)
    //                   ["bi"].set(val6);

    //    CHECK(v.size() == 2);
    //    CHECK(v[0]["pos"].as_float(0) == val1);
    //    CHECK(v[0]["pos"].as_float(1) == val2);
    //    CHECK(v[0]["bi"].as_float(0) == val3);
    //    CHECK(v[1]["pos"].as_float(0) == val4);
    //    CHECK(v[1]["pos"].as_float(1) == val5);
    //    CHECK(v[1]["bi"].as_float(0) == val6);
    //}

    //SECTION("Append data to the array by traversing")
    //{
    //    std::size_t growth{10};
    //    auto form = v.grow(growth);
    //    auto pos = form["pos"];
    //    auto bi  = form["bi"];
    //    float val = 0.5;
    //    
    //    for (std::size_t i = 0; i < growth; ++i)
    //    {
    //        pos.set(val,val);
    //        bi.set(val);
    //        pos.step();
    //        bi.step();
    //    }

    //    CHECK(v.size() == growth);
    //    auto vertex = v[9];
    //    CHECK(vertex["pos"][0].as_float() == val);
    //}
}
