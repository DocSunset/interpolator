#include <catch2/catch.hpp>
#include "test/common.h"
#include "gl/ll/error.h"

#ifdef DEBUG
TEST_CASE("Error reporting", "[gl][error]")
{
    PLATFORM_SETUP();

    using namespace GL::LL;

    SECTION("get_error returns an accurate error code.")
    {
        glCompileShader(42); // no such shader, should return GL_INVALID_VALUE
        auto err = get_error();
        REQUIRE(err == Error::INVALID_VALUE);
    }

    SECTION("get_error returns the first error since the last check.")
    {
        glCompileShader(42); // GL_INVALID_VALUE
        auto p = glCreateProgram();
        glCompileShader(p); // GL_INVALID_OPERATION
        auto err = get_error();
        REQUIRE(err == Error::INVALID_VALUE);
    }

    SECTION("last_error returns only the last generated error.")
    {
        glCompileShader(42); // GL_INVALID_VALUE
        auto p = glCreateProgram();
        glCompileShader(p); // GL_INVALID_OPERATION
        auto err = last_error();
        REQUIRE(err == Error::INVALID_VALUE);
    }


}
#endif
