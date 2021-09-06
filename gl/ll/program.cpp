#include "program.h"
#include "error.h"
#include <iostream>

namespace
{
    using namespace GL::LL;
    bool attach_shader(GLuint& program, GLuint& current_shader, const GLuint& new_shader)
    {
        if (current_shader != 0) 
        {
            if (current_shader == new_shader) 
                return true;
            glDetachShader(program, current_shader);
#ifdef DEBUG
            auto error = last_error();
            if (error != Error::NO_ERROR)
            {
                error_print("Unexpected error after glDetachShader in Program::attach_vertex_shader.\n");
                return false;
            }
#endif
        }
        glAttachShader(program, new_shader);
#ifdef DEBUG
        auto error = last_error();
        if (error != Error::NO_ERROR)
        {
            error_print("Unexpected error after glAttachShader in Program::attach_vertex_shader.\n");
            return false;
        }
#endif
        current_shader = new_shader;
        return true;
    }

    GLint get_program_iv(GLuint handle, GLenum param)
    {
        GLint out;
        glGetProgramiv(handle, param, &out);
#ifdef DEBUG
        if (any_error()) error_print("glGetProgramiv got unexpected error.\n");
#endif
        return out;
    }
}

namespace GL::LL
{
    Program::Program()
        : handle{glCreateProgram()}
        , vertex_shader{0}
        , fragment_shader{0}
        , _attributes{}
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
        vertex_shader = other.vertex_shader;
        fragment_shader = other.fragment_shader;
        _attributes = std::move(other._attributes);
        other.handle = 0;
    }

    Program& Program::operator=(Program&& other)
    {
        glDeleteProgram(handle);
        handle = other.handle;
        vertex_shader = other.vertex_shader;
        fragment_shader = other.fragment_shader;
        _attributes = std::move(other._attributes);
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

    void Program::validate() const
    {
        glValidateProgram(handle);
#ifdef DEBUG
        auto error = last_error();
        if (error == Error::INVALID_OPERATION)
        {
            error_print("Program::link, glLinkProgram, called on active program while transform feedback mode is active.\n");
        }
        else if (error != Error::NO_ERROR)
        {
            error_print("glLinkProgram in Program::link generated unexpected error.\n");
        }
#endif
    }

    bool Program::validation_status() const
    {
        if (handle == 0) 
        {
            error_print("Invalid program.\n");
            return false;
        }
        GLint status;
        glGetProgramiv(handle, GL_VALIDATE_STATUS, &status);
#ifdef DEBUG
        auto error = last_error();
        if (error != Error::NO_ERROR)
            error_print("Program::link_status got unexpected errors.\n");
#endif
        return status == GL_TRUE;
    }

    void Program::link()
    {
        glLinkProgram(handle);
#ifdef DEBUG
        auto error = last_error();
        if (error == Error::INVALID_OPERATION)
        {
            error_print("Program::link, glLinkProgram, called on active program while transform feedback mode is active.\n");
        }
        else if (error != Error::NO_ERROR)
        {
            error_print("glLinkProgram in Program::link generated unexpected error.\n");
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

    void Program::use() const
    {
        if (handle == 0) 
        {
            error_print("Invalid program.\n");
            return;
        }
        glUseProgram(handle);
#ifdef DEBUG
        auto error = last_error();
        if (error == Error::INVALID_OPERATION)
        {
            error_print("Program could not be made part of current state, or transform feedback mode is active and not paused.\n");
        }
        else if (error != Error::NO_ERROR)
        {
            error_print("Program::use got unexpected errors.\n");
        }
#endif
    }

    Program::Program(const char * vertex_source, const char * fragment_source)
        : Program()
    {
        bool failed = false;

        VertexShader v{};
        v.set_source(vertex_source);
        v.compile();
        v.print_info_log();
        if (not v.compile_status()) failed = true;

        FragmentShader f{};
        f.set_source(fragment_source);
        f.compile();
        f.print_info_log();
        if (not f.compile_status()) failed = true;

        if (failed) return;

        attach_vertex_shader(v);
        attach_fragment_shader(f);
        link();
        print_info_log();
        if (not link_status()) return;

        get_attributes();
    }

    Program::operator bool() const
    {
        validate();
        print_info_log();
        return validation_status();
    }

    const AttributeManifest& Program::attributes() const
    {
        return _attributes;
    }

    void Program::get_attributes()
    {
        _attributes = AttributeManifest(*this);
    }

    GLint Program::active_attributes() const
    {
        return get_program_iv(handle, GL_ACTIVE_ATTRIBUTES);
    }

    GLint Program::max_attribute_name_length() const
    {
        return get_program_iv(handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH);
    }
}
