#pragma once

#include <GLES3/gl3.h>

namespace GL::LL
{

    class Shader
    {
        GLuint handle;
        Shader(GLuint other);
    public:
        enum class Type
        {
            Vertex,
            Fragment,
        };

        Shader(Type shader_type);
        void delete_handle();
        operator bool() const;

        // not yet implemented

        //bool is_shader() const;
        void set_source(const GLchar *); // glShaderSource
        void compile(); // glCompileShader

        //// glGetShaderiv
        //Type type() const;
        //bool is_deleted() const;
        bool compile_status() const;
        void print_info_log() const; // glGetShaderiv, glGetShaderInfoLog
        GLchar * source() const; // glGetShaderiv, glGetShaderSource
    };
}
