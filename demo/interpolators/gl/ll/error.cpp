#include "error.h"
#include <iostream>

namespace
{
    using namespace GL::LL;
    Error error_code(GLenum glerror)
    {
#ifdef DEBUG
        switch (glerror)
        {
            case GL_NO_ERROR:
                return Error::NO_ERROR;
            case GL_INVALID_ENUM:
                return Error::INVALID_ENUM;
            case GL_INVALID_VALUE:
                return Error::INVALID_VALUE;
            case GL_INVALID_OPERATION:
                return Error::INVALID_OPERATION;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                return Error::INVALID_FRAMEBUFFER_OPERATION;
            case GL_OUT_OF_MEMORY:
                return Error::OUT_OF_MEMORY;
            default:
                return Error::UNKNOWN_GL_ERROR;
        }
#else
        return Error::NO_ERROR;
#endif
    }

#define STRINGIZE_ERROR_NAME(n) #n

    const char * error_names[static_cast<std::size_t>(Error::N_ERRORS)]
    {
        [static_cast<std::size_t>(Error::NO_ERROR)] = STRINGIZE_ERROR_NAME(NO_ERROR),
        [static_cast<std::size_t>(Error::INVALID_ENUM)] = STRINGIZE_ERROR_NAME(INVALID_ENUM),
        [static_cast<std::size_t>(Error::INVALID_VALUE)] = STRINGIZE_ERROR_NAME(INVALID_VALUE),
        [static_cast<std::size_t>(Error::INVALID_OPERATION)] = STRINGIZE_ERROR_NAME(INVALID_OPERATION),
        [static_cast<std::size_t>(Error::INVALID_FRAMEBUFFER_OPERATION)] = STRINGIZE_ERROR_NAME(INVALID_FRAMEBUFFER_OPERATION),
        [static_cast<std::size_t>(Error::OUT_OF_MEMORY)] = STRINGIZE_ERROR_NAME(OUT_OF_MEMORY),
        [static_cast<std::size_t>(Error::UNKNOWN_GL_ERROR)] = STRINGIZE_ERROR_NAME(UNKNOWN_GL_ERROR),
    };

#undef STRINGIZE_ERROR_NAME

    const char * error_name(Error error)
    {
        return error_names[static_cast<std::size_t>(error)];
    }

    const char * error_name(GLenum glerror)
    {
        return error_name(error_code(glerror));
    }
}
namespace GL::LL
{
    Error get_error()
    {
#ifdef DEBUG
        auto gle = glGetError();
        auto e = error_code(gle);
        if (e != Error::NO_ERROR)
            std::cerr << "GL Error: " << error_name(e) << "\n";
        return e;
#else
        return Error::NO_ERROR;
#endif
    }

    Error last_error(Error prev)
    {
#ifdef DEBUG
        auto next = get_error();
        if (next == Error::NO_ERROR) return prev;
        else return last_error(next);
#else
        return Error::NO_ERROR;
#endif
    }

    void unspecified_error_print(const char * message)
    {
#ifdef DEBUG
        std::cerr << "    Possible non-conformant GL error.\n    " << message;
#else
        return;
#endif
    }

    void error_print(const char * message)
    {
#ifdef DEBUG
        std::cerr << "    " << message;
#else
        return;
#endif
    }
}
