#pragma once

#include <GLES3/gl3.h>

namespace GL::LL
{
    // This class is only meant as a handle; it does not manage data.
    // The target and memory usage hint are fixed at construction.
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
        friend class BufferBinding;
        GLuint handle;
        GLenum target;
        GLenum usage;
    public:
        Buffer(Target t, Usage u);
        Buffer(Buffer&& move);
        ~Buffer();

        operator bool() const;
    };

    // `BufferBinding` provides scoped access to buffer API calls that operate
    // on currently bound buffers.
    class BufferBinding
    {
        const Buffer& b;
    public:
        BufferBinding(const Buffer&);

        // The user is responsible for ensuring that size is non-negative,
        // and that data is nullptr or valid.
        //
        // Furthermore, the user must check for GL_OUT_OF_MEMORY errors later;
        // checking is not performed here as it might degrade performance.
        // Other errors should be impossible by construction.
        void buffer_data(GLsizeiptr size, const void * data);

        GLint parameter(Buffer::Parameter param);
        GLint64 parameter(Buffer::Parameter64 param);
    };

    inline BufferBinding bind(const Buffer& b) {return BufferBinding(b);}
}

