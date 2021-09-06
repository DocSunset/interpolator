#pragma once

#include <GLES3/gl3.h>

namespace GL::LL
{
    // this class is only meant as a handle; it does not manage data
    class Buffer
    {
    public:
        enum class Target
        {
            ARRAY,
            COPY_READ,
            COPY_WRITE,
            ELEMENT_ARRAY,
            PIXEL_PACK,
            PIXEL_UNPACK,
            TRANSFORM_FEEDBACK,
            UNIFORM
        };

        enum class Parameter
        {
            ACCESS_FLAGS,
            MAPPED,
            SIZE,
            USAGE
        };

        enum class Parameter64
        {
            MAP_LENGTH,
            MAP_OFFSET,
            SIZE,
        };

        enum class Usage
        {
            STREAM_DRAW,
            STREAM_READ,
            STREAM_COPY,
            STATIC_DRAW,
            STATIC_READ,
            STATIC_COPY,
            DYNAMIC_DRAW,
            DYNAMIC_READ,
            DYNAMIC_COPY
        };
    private:
        GLuint handle;
        GLenum target;
    public:
        Buffer(Target t);
        ~Buffer();

        void bind() const;
        operator bool() const;
    };
}
