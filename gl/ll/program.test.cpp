#include <catch2/catch.hpp>
#include "test/common.h"
#include "test/shader_sources.h"
#include "gl/ll/program.h"
#include "gl/ll/shader.h"

TEST_CASE("Program attaching shaders", "[gl][program]")
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
        CHECK(p.attached_shaders() == 2);
    }

    SECTION("Attaching a new shader automatically calls glDetachShader to make way")
    {
        Program p{};
        VertexShader v1{};
        VertexShader v2{};

        p.attach_vertex_shader(v1);
        CHECK(p.attach_vertex_shader(v2));
        CHECK(p.attached_shaders() == 1);
    }

    SECTION("Trying to attach the same shader twice skips the repeated error-generating call to glAttachShader.")
    {
        Program p{};
        VertexShader v{};
        p.attach_vertex_shader(v);
        CHECK(p.attach_vertex_shader(v));
        CHECK(p.attached_shaders() == 1);
    }
}

TEST_CASE("Program linking", "[gl][program]")
{
    PLATFORM_SETUP();

    using namespace GL::LL;

    SECTION("Program links successfully given compiled shaders.")
    {
        Program p{};
        VertexShader v{};
        FragmentShader f{};

        CHECK(p.attach_vertex_shader(v));
        CHECK(p.attach_fragment_shader(f));
        CHECK(p.attached_shaders() == 2);

        v.set_source(vertex_source);
        v.compile();
        CHECK(v.compile_status());

        f.set_source(fragment_source);
        f.compile();
        CHECK(f.compile_status());
        f.print_info_log();

        p.link();
        p.print_info_log();
        REQUIRE(p.link_status());
    }

    SECTION("Multiple programs can coexist")
    {
        auto make_ready_program = []()
        {
            Program p{};
            VertexShader v{};
            FragmentShader f{};

            CHECK(p.attach_vertex_shader(v));
            CHECK(p.attach_fragment_shader(f));
            CHECK(p.attached_shaders() == 2);

            v.set_source(vertex_source);
            v.compile();
            CHECK(v.compile_status());

            f.set_source(fragment_source);
            f.compile();
            CHECK(f.compile_status());
            f.print_info_log();

            p.link();
            p.print_info_log();
            REQUIRE(p.link_status());
            return p;
        };
        auto p1 = make_ready_program();
        auto p2 = make_ready_program();
    }

    SECTION("Multiple programs can share shaders")
    {
        VertexShader v{};
        FragmentShader f{};

        v.set_source(vertex_source);
        v.compile();
        CHECK(v.compile_status());

        f.set_source(fragment_source);
        f.compile();
        CHECK(f.compile_status());
        f.print_info_log();

        auto make_ready_program = [&]()
        {
            Program p{};
            p.attach_vertex_shader(v);
            p.attach_fragment_shader(f);
            CHECK(p.attached_shaders() == 2);

            p.link();
            p.print_info_log();
            REQUIRE(p.link_status());
            return p;
        };

        auto p1 = make_ready_program();
        auto p2 = make_ready_program();
    }
}

TEST_CASE("Program validation", "[gl][program]")
{
    PLATFORM_SETUP();
    
    using namespace GL::LL;

    SECTION("Valid program validates successfully")
    {
        Program p{};
        VertexShader v{};
        FragmentShader f{};

        CHECK(p.attach_vertex_shader(v));
        CHECK(p.attach_fragment_shader(f));
        CHECK(p.attached_shaders() == 2);

        v.set_source(vertex_source);
        v.compile();
        CHECK(v.compile_status());

        f.set_source(fragment_source);
        f.compile();
        CHECK(f.compile_status());
        f.print_info_log();

        p.link();
        p.print_info_log();
        CHECK(p.link_status());
        p.validate();
        p.print_info_log();
        REQUIRE(p.validation_status());
    }
}
