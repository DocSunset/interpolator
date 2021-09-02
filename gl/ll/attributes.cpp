#include "attributes.h"
#include <string>

namespace GL::LL
{
    Attribute::Attribute(const char * name, Attribute::Type type)
        : _name{name}, _type{type}
    {
    }

    Attribute::Attribute(GLuint program, GLuint index, char * name_buffer, GLint buffer_size)
    {
    }

    const char * Attribute::name() const {return _name.c_str();}
    Attribute::Type Attribute::type() const {return _type;}
}
