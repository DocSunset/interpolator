#include "program.h"
#include "error.h"

namespace GL::LL
{
    Program::Program()
    {
        handle = glCreateProgram();
#ifdef DEBUG
        if (handle == 0)
            error_print("There was an error creating a program object.\n");
#endif
    }

    Program::Program(Program&& other)
    {
        glDeleteProgram(handle);
        handle = other.handle;
        other.handle = 0;
    }

    Program& Program::operator=(Program&& other)
    {
        glDeleteProgram(handle);
        handle = other.handle;
        other.handle = 0;
        return *this;
    }

    Program::~Program()
    {
        glDeleteProgram(handle);
        handle = 0;
    }

    bool Program::attach_vertex_shader(const VertexShader& shader)
    {
        if (vertex_shader != 0) glDetachShader(handle, vertex_shader);
#ifdef DEBUG
        Error = last_error();
        if (last_error != Error::NO_ERROR)
        {
            error_print("Unexpected error after glDetachShader in Program::attach_vertex_shader.\n");
            return false;
        }
#endif
        glAttachShader(handle, shader);
        vertex_shader = shader.handle;
#ifdef DEBUG
        Error = last_error();
        if (last_error != Error::NO_ERROR)
        {
            error_print("Unexpected error after glAttachShader in Program::attach_vertex_shader.\n");
            return false;
        }
#endif
        return true;
    }

    bool Program::attach_fragment_shader(const FragmentShader& shader)
    {
        if (fragment_shader != 0) glDetachShader(handle, fragment_shader);
#ifdef DEBUG
        Error = last_error();
        if (last_error != Error::NO_ERROR)
        {
            error_print("Unexpected error after glDetachShader in Program::attach_vertex_shader.\n");
            return false;
        }
#endif
        glAttachShader(handle, shader);
        fragment_shader = shader.handle;
#ifdef DEBUG
        Error = last_error();
        if (last_error != Error::NO_ERROR)
        {
            error_print("Unexpected error after glAttachShader in Program::attach_vertex_shader.\n");
            return false;
        }
#endif
        return true;
    }
}
