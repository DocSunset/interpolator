#include "vertex_array.h"
#include <GLES3/gl3.h>

namespace GL::LL
{
    VertexArray::VertexArray()
    {
        glGenVertexArrays(1, &handle);
    }

    VertexArray::~VertexArray()
    {
        glDeleteVertexArrays(1, &handle);
    }

    VertexArray::operator bool() const
    {
#ifdef DEBUG
        return glIsVertexArray(handle);
#else
        return handle != 0;
#endif
    }

    VertexArrayBinding::VertexArrayBinding(const VertexArray& vao)
        : v{vao}
    {
        glBindVertexArray(v.handle);
    }
}
