#pragma once

#include "attribute_type.h"
#include "attribute_element.h"
#include <string>
#include <GLES3/gl3.h>

namespace GL::LL
{
    GLint rows(AttributeType t);

    GLuint columns(AttributeType t);

    std::size_t elements(AttributeType t);

    AttributeElementType element_type(AttributeType t);

    GLenum gl_type(AttributeType t);

    class Attribute
    {
    private:
        friend class AttributeManifest;
        Attribute(GLuint program, GLuint index, char * name_buffer, GLint buffer_size);
        std::string _name;
        AttributeType _type;
        GLuint _location;

    public:
        Attribute(const char * name, AttributeType type);

        bool operator==(const Attribute& other) const;
        bool operator!=(const Attribute& other) const;

        const char * name() const;
        AttributeType type() const;
        GLuint location() const;
        GLint rows() const {return ::GL::LL::rows(_type);}
        GLuint columns() const {return ::GL::LL::columns(_type);}
        std::size_t elements() const {return ::GL::LL::elements(_type);}
        AttributeElementType element_type() const {return ::GL::LL::element_type(_type);}
        GLenum gl_type() const {return ::GL::LL::gl_type(_type);}
    };
}
