#include "attributes.h"
#include "error.h"
#include <string>

namespace
{
    using namespace GL::LL;
    Attribute::Type gl_to_attrib_type(GLenum type)
    {
        switch(type)
        {
            case GL_FLOAT: return Attribute::Type::FLOAT;
            case GL_FLOAT_VEC2: return Attribute::Type::VEC2;
            case GL_FLOAT_VEC3: return Attribute::Type::VEC3;
            case GL_FLOAT_VEC4: return Attribute::Type::VEC4;
            case GL_FLOAT_MAT2: return Attribute::Type::MAT2;
            case GL_FLOAT_MAT3: return Attribute::Type::MAT3;
            case GL_FLOAT_MAT4: return Attribute::Type::MAT4;
            case GL_FLOAT_MAT2x3: return Attribute::Type::MAT2x3;
            case GL_FLOAT_MAT2x4: return Attribute::Type::MAT2x4;
            case GL_FLOAT_MAT3x2: return Attribute::Type::MAT3x2;
            case GL_FLOAT_MAT3x4: return Attribute::Type::MAT3x4;
            case GL_FLOAT_MAT4x2: return Attribute::Type::MAT4x2;
            case GL_FLOAT_MAT4x3: return Attribute::Type::MAT4x3;
            case GL_INT: return Attribute::Type::INT;
            case GL_INT_VEC2: return Attribute::Type::IVEC2;
            case GL_INT_VEC3: return Attribute::Type::IVEC3;
            case GL_INT_VEC4: return Attribute::Type::IVEC4;
            case GL_UNSIGNED_INT: return Attribute::Type::UINT;
            case GL_UNSIGNED_INT_VEC2: return Attribute::Type::UVEC2;
            case GL_UNSIGNED_INT_VEC3: return Attribute::Type::UVEC3;
            case GL_UNSIGNED_INT_VEC4: return Attribute::Type::UVEC4;
            default:
                error_print("Encountered unknown attribute type\n");
                return Attribute::Type::UNKNOWN;
        }
    }
}

namespace GL::LL
{
    Attribute::Attribute(const char * name, Attribute::Type type)
        : _name{name}, _type{type}
    {
    }

    Attribute::Attribute(GLuint program, GLuint index, char * name_buffer, GLint buffer_size)
    {
        GLint size = 0;
        GLenum type = 0;
        glGetActiveAttrib(program, index, buffer_size, nullptr, &size, &type, name_buffer);
#ifdef DEBUG
        auto error = last_error();
        if (error != Error::NO_ERROR) error_print("glGetActiveAttrib returned unexpected error\n");
#endif
        _name = name_buffer;
        _type = gl_to_attrib_type(type);
        error_print("HEY!!!!! In this thing on?\n");
        error_print((std::to_string(size) + std::string("\n")).c_str());
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
