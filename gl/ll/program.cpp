#include "program.h"
#include "error.h"
#include <iostream>

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
#ifdef DEBUG
        Error = last_error();
        if (last_error != Error::NO_ERROR)
        {
            error_print("Unexpected error after glAttachShader in Program::attach_vertex_shader.\n");
            return false;
        }
#endif
        current_shader = new_shader;
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

    int Program::attached_shaders() const
    {
        GLint attached = 0;
        glGetProgramiv(handle, GL_ATTACHED_SHADERS, &attached);
        return attached;
    }

    void Program::link()
    {
        glLinkProgram(handle);
#ifdef DEBUG
        auto error = last_error();
        if (last_error == Error::INVALID_OPERATION)
        {
            error_print("Program::link, glLinkProgram, called on active program while transform feedback mode is active.\n");
        }
        else if (last_error != Error::NO_ERROR)
        {
            error_print("glLinkProgram in Program::link generated unexpected error.\n")
        }
#endif
    }

    bool Program::link_status() const
    {
        if (handle == 0) 
        {
            error_print("Invalid program.\n");
            return false;
        }
        GLint status;
        glGetProgramiv(handle, GL_LINK_STATUS, &status);
#ifdef DEBUG
        auto error = last_error();
        if (error != Error::NO_ERROR)
            error_print("Program::link_status got unexpected errors.\n");
#endif
        return status == GL_TRUE;
    }

    void Program::print_info_log() const
    {
        if (handle == 0) 
        {
            error_print("Invalid program.\n");
            return;
        }
        GLint log_length;
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
        if (log_length <= 0) return;
        GLchar * log = (GLchar *)malloc(log_length);
        GLint length_again;
        glGetProgramInfoLog(handle, log_length, &length_again, log);
        std::cerr << "\n!!! Program info log:\n    " << log << "\n";
        free(log);
#ifdef DEBUG
        if (any_error())
            error_print("Program::link_status got unexpected errors.\n");
#endif
        return;
    }
}
