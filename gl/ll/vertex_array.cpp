#include "vertex_array.h"
#include <GLES3/gl3.h>
#include "error.h"

namespace GL::LL
{
    VertexArray::VertexArray()
    {
        glGenVertexArrays(1, &handle);
#ifdef DEBUG
        if (any_error()) error_print("unexpected error after glGenVertexArrays\n");
#endif
    }

    VertexArray::~VertexArray()
    {
        glDeleteVertexArrays(1, &handle);
#ifdef DEBUG
        if (any_error()) error_print("unexpected error after glDeleteVertexArrays\n");
#endif
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
#ifdef DEBUG
        if (any_error()) error_print("unexpected error after glBindVertexArray\n");
#endif
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

    void VertexArrayBinding::enable_attrib_pointer(const AttributeManifest& manifest, std::size_t idx)
    {
        attrib_pointer(manifest, idx);
        enable_attrib_array(manifest, idx);
    }

    std::string to_string(AttributeElementType elem)
    {
        switch (elem)
        {
            case AttributeElementType::FLOAT: return "GL_FLOAT";
            case AttributeElementType::INT: return "GL_INT";
            case AttributeElementType::UINT: return "GL_UNSIGNED_INT";
        }
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
                        , (void *)manifest.byte_offset_of(idx)
                        );
#ifdef DEBUG
                    if (any_error()) error_print("unexpected error after glVertexAttribPointer\n");
#endif
                    break;
                case AttributeElementType::INT:
                case AttributeElementType::UINT:
                    glVertexAttribIPointer
                        ( attribute.location() + i
                        , attribute.rows()
                        , attribute.gl_type()
                        , (GLsizei)manifest.bytes()
                        , (void *)manifest.byte_offset_of(idx)
                        );
#ifdef DEBUG
                    if (any_error()) error_print("unexpected error after glVertexAttribPointer\n");
#endif
                    break;
            }
        }
    }

    void VertexArrayBinding::enable_attrib_array(const AttributeManifest& manifest, std::size_t idx)
    {
        auto attribute = manifest[idx];
        for (GLuint i = 0; i < attribute.columns(); ++i)
        {
            error_print((std::string("glEnableVertexAttribArray(") + std::to_string(attribute.location() + i) + std::string(")\n")).c_str());
            glEnableVertexAttribArray(attribute.location() + i);
        }
#ifdef DEBUG
        if (any_error()) error_print("unexpected error after glEnableVertexAttribArray\n");
#endif
    }
}
