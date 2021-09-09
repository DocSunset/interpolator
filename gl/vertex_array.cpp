#include "vertex_array.h"

namespace GL
{
    VertexArray::VertexArray(const LL::Program& p)
        : program{p}
        , vao{}
        , _attributes{p}
        , _size{0}
        , _capacity{8}
        , array{(LL::AttributeElement*)malloc(_capacity * _attributes.elements())}
    {
        // set up vertexAttribPointer, Attrib locations, etc.
    }

    std::size_t VertexArray::size() const { return _size; }
    std::size_t VertexArray::capacity() const { return _capacity; }

    VertexForm VertexArray::add_vertex() 
    {
        if ((_size + 1) >= _capacity) reserve(2 * _capacity);
        auto next = array + _size * stride();
        ++_size;
        return VertexForm(_attributes, next);
    }

    void VertexArray::reserve(std::size_t new_cap)
    {
        return;
    }

    const VertexForm VertexArray::operator[](std::size_t i) const
    {
        return VertexForm(_attributes, array + _attributes.elements() * i);
    }

    VertexForm VertexArray::operator[](std::size_t i)
    {
        return VertexForm(_attributes, array + _attributes.elements() * i);
    }

}
