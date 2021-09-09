#pragma once

#include <GLES3/gl3.h>
#include "attribute_manifest.h"

namespace GL::LL
{
    class VertexArray
    {
    private:
        friend class VertexArrayBinding;
        GLuint handle;
    public:
        VertexArray();
        ~VertexArray();
        operator bool() const;
    };

    class VertexArrayBinding
    {
        const VertexArray& v;
    public:
        VertexArrayBinding(const VertexArray& vao);

        void attrib_pointer(const AttributeManifest&, const char * name);
        void attrib_pointer(const AttributeManifest&, std::size_t idx);
        void enable_attrib_array(const AttributeManifest&, const char * name);
        void enable_attrib_array(const AttributeManifest&, std::size_t idx);
        void enable_attrib_pointer(const AttributeManifest&, const char * name);
        void enable_attrib_pointer(const AttributeManifest&, std::size_t idx);
    };

    inline VertexArrayBinding bind(const VertexArray& vao) {return VertexArrayBinding(vao);}
}
