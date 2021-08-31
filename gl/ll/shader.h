#pragma once

#include <GLES3/gl3.h>

namespace GL::LL
{

    /* class Shader
     *
     * This is a wrapper around a shader handle, providing RAII-based lifetime
     * management. RAII alone makes many errors impossible, since most gl shader
     * calls can fail with INVALID_VALUE or INVALID_OPERATION if the GLuint
     * handle is not valid, which is avoided entirely with RAII. Some calls
     * can still fail with INVALID_ENUM, but these are also avoided here by
     * hard coding only valid enum values.
     *
     * Helper methods are provided for common use-cases. Most errors should not
     * be possible when using the helper methods.
     */
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

        Shader& operator=(Shader&& other);

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

    class VertexShader : public Shader
    {
    public:
        VertexShader() : Shader(Shader::Type::Vertex) {}
        VertexShader(VertexShader&& other);
    };
}
