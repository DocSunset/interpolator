#pragma once

#include <GLES3/gl3.h>

namespace GL::LL
{

    class Shader
    {
        GLuint handle;
    public:
        enum class Type
        {
            Vertex,
            Fragment,
        };

        Shader(Type shader_type);
        Shader() = delete;
        Shader(const Shader& other) = delete;
        Shader(Shader&& other);
        ~Shader();

        operator bool() const;

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
