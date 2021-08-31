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

    SECTION("last_error flushes all errors.")
    {
        glCompileShader(42); // GL_INVALID_VALUE
        glCompileShader(42); // GL_INVALID_VALUE
        glCompileShader(42); // GL_INVALID_VALUE
        last_error();
        REQUIRE(last_error() == Error::NO_ERROR);
    }

    SECTION("any_error returns true if there have been any errors.")
    {
        glCompileShader(42); // GL_INVALID_VALUE
        REQUIRE(any_error());
        glCreateProgram();
        glCompileShader(42); // GL_INVALID_VALUE
        glCreateProgram();
        REQUIRE(any_error());
    }

    SECTION("any_error flushes all errors.")
    {
        glCompileShader(42); // GL_INVALID_VALUE
        glCompileShader(42); // GL_INVALID_VALUE
        glCompileShader(42); // GL_INVALID_VALUE
        any_error();
        REQUIRE(not any_error());
    }


}
#endif
