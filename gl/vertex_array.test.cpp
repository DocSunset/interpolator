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
        auto form = v.attributes();
        CHECK(form.has("pos"));
        CHECK(form.has("bi"));

        SECTION("Derive vertices from the form")
        {
            float val = 0.5;
            form["pos"].set(val, val)
                 ["bi"].set(val);
            CHECK(form["pos"][0].as_float() == val);
            CHECK(form["bi"][0].as_float() == val);
        }
    }

    //SECTION("Append data to the array by modifying forms")
    //{
    //    v.add_vertex()["pos"].set(1.0,1.0)
    //                   ["bi"].set(1.0);

    //    REQUIRE(v[0]["pos"][0].as_float() == 1.0);
    //    REQUIRE(v[1]["bi"][0].as_float() == 1.0);
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
    //    CHECK(v[9]["pos"][0].as_float() == val);
    //}
}
