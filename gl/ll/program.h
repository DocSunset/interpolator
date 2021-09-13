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
    protected:
        friend class AttributeManifest;
        GLuint handle;
        GLuint vertex_shader;
        GLuint fragment_shader;
    public:
        GLuint gl_handle() const {return handle;}
        Program();
        Program(const char * vertex_source, const char * fragment_source);

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

        void validate() const;
        bool validation_status() const;

        void print_info_log() const;

        void use() const;

        operator bool() const;

        GLint active_attributes() const;
        GLint max_attribute_name_length() const;
    };
}
