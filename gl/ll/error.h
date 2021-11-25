#pragma once

#include <GLES3/gl3.h>

namespace GL::LL
{
    enum class Error
    {
        NO_ERROR,
        INVALID_ENUM,
        INVALID_VALUE,
        INVALID_OPERATION,
        INVALID_FRAMEBUFFER_OPERATION,
        OUT_OF_MEMORY,
        UNKNOWN_GL_ERROR,
        N_ERRORS,
    };

    Error always_get_error(); // checks for error even on release build

    Error get_error(); // debug-only error check

    Error always_last_error(Error prev = Error::NO_ERROR);

    Error last_error(Error prev = Error::NO_ERROR);

    void unspecified_error_print(const char * message);

    void error_print(const char * message);

    bool any_error();

    bool always_any_error();
}
