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

    bool Attribute::operator==(const Attribute& other) const
    {
        if (_name != other._name) return false;
        if (_type != other._type) return false;
        return true;
    }

    const char * Attribute::name() const {return _name.c_str();}
    Attribute::Type Attribute::type() const {return _type;}
}
