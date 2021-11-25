#include <catch2/catch.hpp>
#include "test/common.h"
#include "gl/ll/sampler.h"

TEST_CASE("Sampler Objects", "[gl][sampler]")
{
    PLATFORM_SETUP();
    using namespace GL::LL;

    SECTION("Sampler is valid before first binding, and valid after.")
    {
        auto s = Sampler();
        CHECK(bool(s));
        SECTION("Sampler is valid after binding")
        {
            auto binding = bind(s, 0);
            CHECK(bool(s));
            SECTION("Sampler is bound after binding")
            {
                CHECK(Sampler::current_binding() == s.gl_handle());
            }
            SECTION("set_wrap")
            {
                SECTION("wrap mode has expected default value")
                {
                    CHECK(binding.get_wrap(Sampler::WrapAxis::S) == Sampler::WrapMode::REPEAT);
                }
                binding.set_wrap(Sampler::WrapAxis::S, Sampler::WrapMode::MIRRORED_REPEAT);
                CHECK(binding.get_wrap(Sampler::WrapAxis::S) == Sampler::WrapMode::MIRRORED_REPEAT);
            }
            SECTION("set_filter_min")
            {
                SECTION("min filter has expected default value")
                {
                    CHECK(binding.get_min_filter() == Sampler::MinFilterMode::NEAREST_MIPMAP_LINEAR);
                }
                binding.set_min_filter(Sampler::MinFilterMode::NEAREST);
                CHECK(binding.get_min_filter() == Sampler::MinFilterMode::NEAREST); 
            }
            SECTION("set_filter_mag")
            {
                SECTION("mag filter has expected default value")
                {
                    CHECK(binding.get_mag_filter() == Sampler::MagFilterMode::LINEAR);
                }
                binding.set_mag_filter(Sampler::MagFilterMode::NEAREST);
                CHECK(binding.get_mag_filter() == Sampler::MagFilterMode::NEAREST); 
            }
        }
    }
}
