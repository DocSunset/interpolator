#include "vertex_array.h"

namespace GL
{
    VertexArray::VertexArray(const LL::Program& p)
        : program{p}
        , vao{}
        , _attributes{p}
    {
        _size = 0;
        _capacity = 0;
        // set up vertexAttribPointer, Attrib locations, etc.
    }

    VertexForm VertexArray::attributes() const
    {
        return VertexForm(_attributes);
    }

    std::size_t VertexArray::size() const
    {
        return 0;
    }

    std::size_t VertexArray::capacity() const
    {
        return 0;
    }
}
