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
        : _name{name}, _type{type}, _location{0}
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
        _location = glGetAttribLocation(program, _name.c_str());
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
    GLuint Attribute::location() const {return _location;}

    GLint rows(AttributeType _type)
    {
        switch (_type)
        {
            case AttributeType::MAT2:
            case AttributeType::MAT2x3:
            case AttributeType::MAT2x4:
                return 2;
            case AttributeType::MAT3:
            case AttributeType::MAT3x2:
            case AttributeType::MAT3x4:
                return 3;
            case AttributeType::MAT4:
            case AttributeType::MAT4x2:
            case AttributeType::MAT4x3:
                return 4;
            case AttributeType::FLOAT:
            case AttributeType::VEC2:
            case AttributeType::VEC3:
            case AttributeType::VEC4:
            case AttributeType::INT:
            case AttributeType::IVEC2:
            case AttributeType::IVEC3:
            case AttributeType::IVEC4:
            case AttributeType::UINT:
            case AttributeType::UVEC2:
            case AttributeType::UVEC3:
            case AttributeType::UVEC4:
                return elements(_type);
            default:
                error_print("Encountered unknown attribute type in rows\n");
                return 0;
        }
    }

    GLuint columns(AttributeType _type)
    {
        switch (_type)
        {
            case AttributeType::MAT2:
            case AttributeType::MAT3x2:
            case AttributeType::MAT4x2:
                return 2;
            case AttributeType::MAT3:
            case AttributeType::MAT2x3:
            case AttributeType::MAT4x3:
                return 3;
            case AttributeType::MAT4:
            case AttributeType::MAT2x4:
            case AttributeType::MAT3x4:
                return 4;
            case AttributeType::FLOAT:
            case AttributeType::VEC2:
            case AttributeType::VEC3:
            case AttributeType::VEC4:
            case AttributeType::INT:
            case AttributeType::IVEC2:
            case AttributeType::IVEC3:
            case AttributeType::IVEC4:
            case AttributeType::UINT:
            case AttributeType::UVEC2:
            case AttributeType::UVEC3:
            case AttributeType::UVEC4:
                return 1;
            default:
                error_print("Encountered unknown attribute type in columns\n");
                return 0;
        }
    }

    std::size_t elements(AttributeType _type)
    {
        switch (_type)
        {
            case AttributeType::FLOAT: return 1;
            case AttributeType::VEC2: return 2;
            case AttributeType::VEC3: return 3;
            case AttributeType::VEC4: return 4;
            case AttributeType::MAT2: return 2 * 2;
            case AttributeType::MAT3: return 3 * 3;
            case AttributeType::MAT4: return 4 * 4;
            case AttributeType::MAT2x3: return 2 * 3;
            case AttributeType::MAT2x4: return 2 * 4;
            case AttributeType::MAT3x2: return 3 * 2;
            case AttributeType::MAT3x4: return 3 * 4;
            case AttributeType::MAT4x2: return 4 * 2;
            case AttributeType::MAT4x3: return 4 * 3;
            case AttributeType::INT: return 1;
            case AttributeType::IVEC2: return 2;
            case AttributeType::IVEC3: return 3;
            case AttributeType::IVEC4: return 4;
            case AttributeType::UINT: return 1;
            case AttributeType::UVEC2: return 2;
            case AttributeType::UVEC3: return 3;
            case AttributeType::UVEC4: return 4;
            default:
                error_print("Encountered unknown attribute type in elements\n");
                return 0;
        }
    }

    AttributeElementType element_type(AttributeType _type)
    {
        switch (_type)
        {
            case AttributeType::FLOAT:
            case AttributeType::VEC2:
            case AttributeType::VEC3:
            case AttributeType::VEC4:
            case AttributeType::MAT2:
            case AttributeType::MAT3:
            case AttributeType::MAT4:
            case AttributeType::MAT2x3:
            case AttributeType::MAT2x4:
            case AttributeType::MAT3x2:
            case AttributeType::MAT3x4:
            case AttributeType::MAT4x2:
            case AttributeType::MAT4x3:
                return AttributeElementType::FLOAT;
            case AttributeType::INT:
            case AttributeType::IVEC2:
            case AttributeType::IVEC3:
            case AttributeType::IVEC4:
                return AttributeElementType::INT;
            case AttributeType::UINT:
            case AttributeType::UVEC2:
            case AttributeType::UVEC3:
            case AttributeType::UVEC4:
                return AttributeElementType::UINT;
            default:
                error_print("Encountered unknown attribute type in element_type\n");
                return AttributeElementType::FLOAT;
        }
    }

    GLenum gl_type(AttributeType _type)
    {
        switch (_type)
        {
            case AttributeType::FLOAT:
            case AttributeType::VEC2:
            case AttributeType::VEC3:
            case AttributeType::VEC4:
            case AttributeType::MAT2:
            case AttributeType::MAT3:
            case AttributeType::MAT4:
            case AttributeType::MAT2x3:
            case AttributeType::MAT2x4:
            case AttributeType::MAT3x2:
            case AttributeType::MAT3x4:
            case AttributeType::MAT4x2:
            case AttributeType::MAT4x3:
                return GL_FLOAT;
            case AttributeType::INT:
            case AttributeType::IVEC2:
            case AttributeType::IVEC3:
            case AttributeType::IVEC4:
                return GL_INT;
            case AttributeType::UINT:
            case AttributeType::UVEC2:
            case AttributeType::UVEC3:
            case AttributeType::UVEC4:
                return GL_UNSIGNED_INT;
            default:
                error_print("Encountered unknown attribute type in gl_type\n");
                return GL_FLOAT;
        }
    }
}
