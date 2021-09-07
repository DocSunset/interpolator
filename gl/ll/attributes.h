#pragma once

#include "attribute_type.h"
#include "attribute_element.h"
#include <string>
#include <GLES3/gl3.h>

namespace GL::LL
{
    std::size_t elements(AttributeType t);

    AttributeElementType element_type(AttributeType t);

    class Attribute
    {
    private:
        friend class AttributeManifest;
        Attribute(GLuint program, GLuint index, char * name_buffer, GLint buffer_size);
        std::string _name;
        AttributeType _type;

    public:
        Attribute(const char * name, AttributeType type);

        bool operator==(const Attribute& other) const;
        bool operator!=(const Attribute& other) const;

        const char * name() const;
        AttributeType type() const;

        std::size_t elements() const {return ::GL::LL::elements(_type);}
        AttributeElementType element_type() const {return ::GL::LL::element_type(_type);}
    };
}
