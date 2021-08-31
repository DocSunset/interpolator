#pragma once

#include <GLES3/gl3.h>
#include "shader.h"

namespace GL::LL
{
    /* class Program
     *
     * This is an RAII wrapper around a program handle with helper methods for
     * common uses. Most errors are avoided when using the helper methods.
     * See class Shader for a more elaborate explanation.
     */
    class Program
    {
        GLuint handle;
        GLuint vertex_shader;
        GLuint fragment_shader;
    public:
        Program();

        Program(Program&& other);
        Program& operator=(Program&& other);

        Program(const Program& other) = delete;
        Program& operator=(const Program& other) = delete;

        ~Program();

        bool attach_vertex_shader(const VertexShader& shader);
        bool attach_fragment_shader(const FragmentShader& shader);

        int attached_shaders() const;

        void link();
        bool link_status() const;

        void validate();
        bool validation_status() const;

        void print_info_log() const;
    };
}
