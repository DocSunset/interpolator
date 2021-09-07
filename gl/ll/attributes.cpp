#include "attributes.h"
#include "error.h"
#include <string>

namespace
{
    using namespace GL::LL;
    AttributeType gl_to_attrib_type(GLenum type)
    {
        switch(type)
        {
            case GL_FLOAT: return AttributeType::FLOAT;
            case GL_FLOAT_VEC2: return AttributeType::VEC2;
            case GL_FLOAT_VEC3: return AttributeType::VEC3;
            case GL_FLOAT_VEC4: return AttributeType::VEC4;
            case GL_FLOAT_MAT2: return AttributeType::MAT2;
            case GL_FLOAT_MAT3: return AttributeType::MAT3;
            case GL_FLOAT_MAT4: return AttributeType::MAT4;
            case GL_FLOAT_MAT2x3: return AttributeType::MAT2x3;
            case GL_FLOAT_MAT2x4: return AttributeType::MAT2x4;
            case GL_FLOAT_MAT3x2: return AttributeType::MAT3x2;
            case GL_FLOAT_MAT3x4: return AttributeType::MAT3x4;
            case GL_FLOAT_MAT4x2: return AttributeType::MAT4x2;
            case GL_FLOAT_MAT4x3: return AttributeType::MAT4x3;
            case GL_INT: return AttributeType::INT;
            case GL_INT_VEC2: return AttributeType::IVEC2;
            case GL_INT_VEC3: return AttributeType::IVEC3;
            case GL_INT_VEC4: return AttributeType::IVEC4;
            case GL_UNSIGNED_INT: return AttributeType::UINT;
            case GL_UNSIGNED_INT_VEC2: return AttributeType::UVEC2;
            case GL_UNSIGNED_INT_VEC3: return AttributeType::UVEC3;
            case GL_UNSIGNED_INT_VEC4: return AttributeType::UVEC4;
            default:
                error_print("Encountered unknown attribute type\n");
                return AttributeType::UNKNOWN;
        }
    }
}

namespace GL::LL
{
    Attribute::Attribute(const char * name, AttributeType type)
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
        if (size != 1) error_print("glGetActiveAttrib returned a size other than 1:"), error_print(std::to_string(size).c_str()), error_print("\n");
#endif
        _name = name_buffer;
        _type = gl_to_attrib_type(type);
    }

    bool Attribute::operator==(const Attribute& other) const
    {
        if (_name != other._name) return false;
        if (_type != other._type) return false;
        return true;
    }

    bool Attribute::operator!=(const Attribute& other) const
    {
        return not (*this == other);
    }

    const char * Attribute::name() const {return _name.c_str();}
    AttributeType Attribute::type() const {return _type;}

    std::size_t Attribute::bytes() const
    {
        switch (_type)
        {
            case AttributeType::FLOAT: return sizeof(GLfloat);
            case AttributeType::VEC2: return 2 * sizeof(GLfloat);
            case AttributeType::VEC3: return 3 * sizeof(GLfloat);
            case AttributeType::VEC4: return 4 * sizeof(GLfloat);
            case AttributeType::MAT2: return 2 * 2 * sizeof(GLfloat);
            case AttributeType::MAT3: return 3 * 3 * sizeof(GLfloat);
            case AttributeType::MAT4: return 4 * 4 * sizeof(GLfloat);
            case AttributeType::MAT2x3: return 2 * 3 * sizeof(GLfloat);
            case AttributeType::MAT2x4: return 2 * 4 * sizeof(GLfloat);
            case AttributeType::MAT3x2: return 3 * 2 * sizeof(GLfloat);
            case AttributeType::MAT3x4: return 3 * 4 * sizeof(GLfloat);
            case AttributeType::MAT4x2: return 4 * 2 * sizeof(GLfloat);
            case AttributeType::MAT4x3: return 4 * 3 * sizeof(GLfloat);
            case AttributeType::INT: return sizeof(GLfloat);
            case AttributeType::IVEC2: return 2 * sizeof(GLfloat);
            case AttributeType::IVEC3: return 3 * sizeof(GLfloat);
            case AttributeType::IVEC4: return 4 * sizeof(GLfloat);
            case AttributeType::UINT: return sizeof(GLfloat);
            case AttributeType::UVEC2: return 2 * sizeof(GLfloat);
            case AttributeType::UVEC3: return 3 * sizeof(GLfloat);
            case AttributeType::UVEC4: return 4 * sizeof(GLfloat);
            default:
                error_print("Encountered unknown attribute type\n");
                return 0;
        }
    }
}
