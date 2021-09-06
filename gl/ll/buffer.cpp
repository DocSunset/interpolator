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
}

namespace GL::LL
{
    Buffer::Buffer(Target t)
        : target{target_to_gl(t)}
    {
        glGenBuffers(1, &handle);
        if (any_error()) error_print("unexpected error after glGenBuffers.\n");
    }

    Buffer::~Buffer()
    {
        glDeleteBuffers(1, &handle);
    }

    void Buffer::bind() const
    {
        glBindBuffer(target, handle);
    }

    Buffer::operator bool() const
    {
#ifdef DEBUG
        return glIsBuffer(handle);
#else
        return handle != 0;
#endif
    }
}
