#include "program.h"
#include "error.h"

namespace
{
    bool attach_shader(GLuint& program, GLuint& current_shader, const GLuint& new_shader)
    {
        if (current_shader != 0) 
        {
            if (current_shader == new_shader) 
                return true;
            glDetachShader(program, current_shader);
        }
#ifdef DEBUG
        Error = last_error();
        if (last_error != Error::NO_ERROR)
        {
            error_print("Unexpected error after glDetachShader in Program::attach_vertex_shader.\n");
            return false;
        }
#endif
        glAttachShader(program, new_shader);
        current_shader = new_shader;
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
        return attach_shader(handle, vertex_shader, shader.handle);
    }

    bool Program::attach_fragment_shader(const FragmentShader& shader)
    {
        return attach_shader(handle, fragment_shader, shader.handle);
    }
}
