#pragma once

namespace GL::LL
{
    enum class AttributeType
    {
        FLOAT,  VEC2,  VEC3,  VEC4,
        INT,   IVEC2, IVEC3, IVEC4,
        UINT,  UVEC2, UVEC3, UVEC4,
        MAT2,   MAT2x3, MAT2x4,
        MAT3x2, MAT3,   MAT3x4,
        MAT4x2, MAT4x3, MAT4,
        UNKNOWN,
    };
}
