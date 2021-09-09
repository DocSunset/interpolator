#pragma once
#include <GLES3/gl3.h>

namespace GL::LL
{
    union AttributeElement
    {
        GLfloat f;
        GLint i;
        GLuint u;
    };

    enum class AttributeElementType
    {
        FLOAT,
        INT,
        UINT,
    };
}
