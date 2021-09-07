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
    VertexForm VertexArray::add_vertex() { return grow(1); }

    VertexForm VertexArray::grow(std::size_t i)
    {
        auto arr = array;
        //if ((_size + i) < _capacity)
        //{
        //    arr = _size;
        //    _size += i;
        //}
        //else realloc array
        return VertexForm(_attributes, arr);
    }

    const VertexForm VertexArray::operator[](std::size_t i) const
    {
        return VertexForm(_attributes, array);
    }

    VertexForm VertexArray::operator[](std::size_t i)
    {
        return static_cast<const VertexArray>(*this)[i];
    }

}
