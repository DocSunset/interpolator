#include <catch2/catch.hpp>
#include "test/common.h"
#include "gl/ll/texture.h"

TEST_CASE("Texture Objects", "[gl][texture]")
{
    PLATFORM_SETUP();
    using namespace GL::LL;

    SECTION("Texture is invalid before first binding, valid after.")
    {
        auto t = Texture(Texture::Target::TEX_2D);
        CHECK(not t);
        SECTION("Texture is valid after binding")
        {
            auto binding = bind(t, 0);
            CHECK(bool(t));
            SECTION("Texture is bound after binding")
            {
                CHECK(Texture::current_binding(Texture::Target::TEX_2D) == t.gl_handle());
            }
        }
    }

    SECTION("get_active_texture returns TEXTURE0 by default.")
    {
        auto active = Texture::get_active_texture_unit();
        CHECK(active == 0);
    }

    SECTION("max_texture_units is at least 32")
    {
        CHECK(Texture::max_combined_texture_image_units() >= 32);
    }

    SECTION("setting active texture")
    {
        unsigned int expected = 1;
        Texture::set_active_texture_unit(expected);
        auto active = Texture::get_active_texture_unit();
        CHECK(active == expected);
    }
}
