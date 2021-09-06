#include "buffer.h"
#include "error.h"
#include <cstdlib>

namespace
{
    using namespace GL::LL;
    constexpr GLenum target_to_gl(Buffer::Target t)
    {
        switch (t)
        {
            case Buffer::Target::ARRAY: 
                       return GL_ARRAY_BUFFER;
            case Buffer::Target::COPY_READ: 
                       return GL_COPY_READ_BUFFER;
            case Buffer::Target::COPY_WRITE: 
                       return GL_COPY_WRITE_BUFFER;
            case Buffer::Target::ELEMENT_ARRAY: 
                       return GL_ELEMENT_ARRAY_BUFFER;
            case Buffer::Target::PIXEL_PACK: 
                       return GL_PIXEL_PACK_BUFFER;
            case Buffer::Target::PIXEL_UNPACK: 
                       return GL_PIXEL_UNPACK_BUFFER;
            case Buffer::Target::TRANSFORM_FEEDBACK: 
                       return GL_TRANSFORM_FEEDBACK_BUFFER;
            case Buffer::Target::UNIFORM: 
                       return GL_UNIFORM_BUFFER;
            default:
#ifdef DEBUG
                error_print("unrecognized Buffer::Target?!\n");
                exit(1);
#else
                return GL_ARRAY_BUFFER;
#endif
        }
    }

    constexpr GLenum usage_to_gl(Buffer::Usage u)
    {
        switch (u)
        {
            case Buffer::Usage::STREAM_DRAW:  return GL_STREAM_DRAW;
            case Buffer::Usage::STREAM_READ:  return GL_STREAM_READ;
            case Buffer::Usage::STREAM_COPY:  return GL_STREAM_COPY;
            case Buffer::Usage::STATIC_DRAW:  return GL_STATIC_DRAW;
            case Buffer::Usage::STATIC_READ:  return GL_STATIC_READ;
            case Buffer::Usage::STATIC_COPY:  return GL_STATIC_COPY;
            case Buffer::Usage::DYNAMIC_DRAW: return GL_DYNAMIC_DRAW;
            case Buffer::Usage::DYNAMIC_READ: return GL_DYNAMIC_READ;
            case Buffer::Usage::DYNAMIC_COPY: return GL_DYNAMIC_COPY;
            default:
#ifdef DEBUG
                error_print("unrecognized Buffer::Target?!\n");
                exit(1);
#else
                return GL_DYNAMIC_DRAW;
#endif
        }
    }
}

namespace GL::LL
{
    Buffer::Buffer(Target t, Usage u)
        : target{target_to_gl(t)}
        , usage{usage_to_gl(u)}
    {
        glGenBuffers(1, &handle);
        if (any_error()) error_print("unexpected error after glGenBuffers.\n");
    }

    Buffer::~Buffer()
    {
        glDeleteBuffers(1, &handle);
    }

    Buffer::operator bool() const
    {
#ifdef DEBUG
        return glIsBuffer(handle);
#else
        return handle != 0;
#endif
    }

    BufferBinding::BufferBinding(const Buffer& buffer)
        : b{buffer}
    {
        glBindBuffer(b.target, b.handle);
    }

    void BufferBinding::buffer_data(GLsizeiptr size, const void * data)
    {
    }
}
