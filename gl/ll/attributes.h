#pragma once

#include <string>
#include <GLES3/gl3.h>

namespace GL::LL
{
    class Attribute
    {
    public:
        enum class Type
        {
            FLOAT,  VEC2,  VEC3,  VEC4,
            INT,   IVEC2, IVEC3, IVEC4,
            UINT,  UVEC2, UVEC3, UVEC4,
            MAT2,   MAT2x3, MAT2x4,
            MAT3x2, MAT3,   MAT3x4,
            MAT4x2, MAT4x3, MAT4,
        };

    private:
        Attribute(GLuint program, GLuint index, char * name_buffer, GLint buffer_size);
        std::string _name;
        Type _type;

    public:
        Attribute(const char * name, Type type);

        bool operator==(const Attribute& other) const;

        const char * name() const;
        Type type() const;
    };
}
