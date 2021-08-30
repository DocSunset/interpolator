#include "shader.h"
#include "error.h"
#include <cstdlib>
#include <iostream>

namespace GL::LL
{
    /* according to the spec, glCreateShader can fail if an invalid enum is
     * given, which is not possible since our Shader::Type enum only supports
     * Vertex or Fragment as values.
     *
     * The spec states that "if an error occurs, zero will be returned", and
     * the only possible error it identifies is an invalid enum. The handle
     * should therefore always be non-zero, and if zero is returned it
     * may indicate a non-conformant implementation.
     */
    Shader::Shader(Type shader_type)
    {
        if (shader_type == Type::Vertex)
        {
            handle = glCreateShader(GL_VERTEX_SHADER);
        }
        else if (shader_type == Type::Fragment)
        {
            handle = glCreateShader(GL_FRAGMENT_SHADER);
        }

#ifdef DEBUG
        if (handle == 0)
        {
            error_print("Error in Shader constructor");
            auto error = last_error();
        }
#endif
    }

    Shader::Shader(Shader&& other)
    {
        this->handle = other.handle;
    }

    /* It is assumed that the shader handle is valid or zero (which is silently ignored).
     *
     * Consequently, no errors should be possible.
     */
    Shader::~Shader()
    {
        glDeleteShader(handle);
        handle = 0;
#ifdef DEBUG
        auto error = last_error();
        if (error != Error::NO_ERROR)
        {
            error_print("glDeleteShader received unexpected GL error code!\n");
        }
#endif
    }

    Shader::operator bool() const
    {
#ifdef DEBUG
        if (handle == 0) return false;
        GLint dummy;
        glGetShaderiv(handle, GL_SHADER_TYPE, &dummy);
        auto error = get_error();
        switch (error)
        {
            case Error::INVALID_VALUE:
                error_print("Shader handle is not a valid object generated by OpenGL.\n");
                return false;
            case Error::INVALID_OPERATION:
                error_print("Shader handle does not refer to a shader.\n");
                return false;
            case Error::INVALID_ENUM:
                unspecified_error_print("glGetShaderiv called with GL_SHADER_TYPE.\n");
                return true;
            case Error::NO_ERROR:
                return true;
            default:
                unspecified_error_print("glGetShaderiv received unspecified GL error code!\n");
                return true;
        }
#else
        return true;
#endif
    }


    /* pass a single null-terminated string of source code
     * invalid value due to bad handle should be impossible
     * invalid value due to zero count should be impossible
     * invalid operation due to non-shader handle should be impossible
     */
    void Shader::set_source(const GLchar * source)
    {
        GLsizei count = 1;
        const GLchar ** sources = &source;
        const GLint* lengths = nullptr;
        glShaderSource(handle, count, sources, lengths);

#ifdef DEBUG
        auto error = last_error();
        switch (error)
        {
            case Error::INVALID_VALUE:
                unspecified_error_print("glShaderSource got INVALID_VALUE despite valid values!\n");
                break;
            case Error::INVALID_OPERATION:
                error_print("glShaderSource supposedly called with non-shader handle.\n");
                break;
            case Error::NO_ERROR:
                break;
            default:
                unspecified_error_print("glShaderSource got an unexpected error code.\n");
        }
#endif
    }

    /* As above, handle must be a valid shader handle, and all enums are valid,
     * so no errors should be possible
     */
    GLchar * Shader::source() const
    {
        GLint source_length;
        glGetShaderiv(handle, GL_SHADER_SOURCE_LENGTH, &source_length);
        if (source_length <= 0) return nullptr;
        GLchar * source = (GLchar *)malloc(source_length);
        GLint length_again;
        glGetShaderSource(handle, source_length, &length_again, source);
#ifdef DEBUG
        auto error = last_error();
        if (error != Error::NO_ERROR)
            error_print("Shader::source got unexpected errors.\n");
#endif
        return source;
    }

    void Shader::compile()
    {
        glCompileShader(handle);
#ifdef DEBUG
        auto error = last_error();
        if (error != Error::NO_ERROR)
            error_print("Shader::compile got unexpected errors.\n");
#endif
    }

    bool Shader::compile_status() const
    {
        GLint status;
        glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
#ifdef DEBUG
        auto error = last_error();
        if (error != Error::NO_ERROR)
            error_print("Shader::compile_status got unexpected errors.\n");
#endif
        return status == GL_TRUE;
    }

    void Shader::print_info_log() const
    {
        GLint log_length;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
        if (log_length <= 0) return;
        GLchar * log = (GLchar *)malloc(log_length);
        GLint length_again;
        glGetShaderInfoLog(handle, log_length, &length_again, log);
        std::cerr << "Shader info log:\n    " << log;
        free(log);
#ifdef DEBUG
        auto error = last_error();
        if (error != Error::NO_ERROR)
            error_print("Shader::compile_status got unexpected errors.\n");
#endif
        return;
    }
}
