#include "vertex_array.h"
#include <GLES3/gl3.h>

namespace GL::LL
{
    VertexArray::VertexArray()
    {
        glGenVertexArrays(1, &handle);
    }

    VertexArray::~VertexArray()
    {
        glDeleteVertexArrays(1, &handle);
    }

    VertexArray::operator bool() const
    {
#ifdef DEBUG
        return glIsVertexArray(handle);
#else
        return handle != 0;
#endif
    }

    VertexArrayBinding::VertexArrayBinding(const VertexArray& vao)
        : v{vao}
    {
        glBindVertexArray(v.handle);
    }

    void VertexArrayBinding::attrib_pointer(const AttributeManifest& manifest, const char * name)
    {
        return attrib_pointer(manifest, manifest.index_of(name));
    }

    void VertexArrayBinding::enable_attrib_array(const AttributeManifest& manifest, const char * name)
    {
        return enable_attrib_array(manifest, manifest.index_of(name));
    }

    void VertexArrayBinding::enable_attrib_pointer(const AttributeManifest& manifest, const char * name)
    {
        return enable_attrib_pointer(manifest, manifest.index_of(name));
    }

    void VertexArrayBinding::attrib_pointer(const AttributeManifest& manifest, std::size_t idx)
    {
        auto attribute = manifest[idx];
        for (GLuint i = 0; i < attribute.columns(); ++i)
        {
            switch (attribute.element_type())
            {
                case AttributeElementType::FLOAT:
                    glVertexAttribPointer
                        ( attribute.location() + i
                        , attribute.rows()
                        , attribute.gl_type()
                        , false
                        , (GLsizei)manifest.bytes()
                        , (void *)manifest.offset_of(idx)
                        );
                case AttributeElementType::INT:
                case AttributeElementType::UINT:
                    glVertexAttribIPointer
                        ( attribute.location() + i
                        , attribute.rows()
                        , attribute.gl_type()
                        , (GLsizei)manifest.bytes()
                        , (void *)manifest.offset_of(idx)
                        );
                    break;
            }
        }
    }

    void VertexArrayBinding::enable_attrib_array(const AttributeManifest& manifest, std::size_t idx)
    {
        auto attribute = manifest[idx];
        for (GLuint i = 0; i < attribute.columns(); ++i)
        {
            glEnableVertexAttribArray(attribute.location() + i);
        }
    }

    void VertexArrayBinding::enable_attrib_pointer(const AttributeManifest& manifest, std::size_t idx)
    {
        attrib_pointer(manifest, idx);
        enable_attrib_array(manifest, idx);
    }

}
