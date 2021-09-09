#include "vertex_array.h"

namespace GL
{
    VertexArray::VertexArray(const LL::Program& p)
        : program{p}
        , vao{}
        , _attributes{p}
        , _size{0}
        , _capacity{8}
        , array{(LL::AttributeElement*)malloc(_capacity * _attributes.bytes())}
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
        if (new_cap <= _capacity) return;
        decltype(array) new_array = (decltype(array))realloc((void *)array, new_cap * _attributes.bytes());
        if (new_array == nullptr)
        {
            LL::error_print("failed to allocate memory in VertexArray::reserve\n");
            LL::error_print("caller requested ");
            LL::error_print(std::to_string(new_cap).c_str());
            LL::error_print(" vertices\n");
            LL::error_print("vertex size is ");
            LL::error_print(std::to_string(_attributes.elements()).c_str());
            LL::error_print("attribute elements.\n");
            exit(1);
        }
        array = new_array;
        _capacity = new_cap;
        return;
    }

    const VertexForm VertexArray::operator[](std::size_t i) const
    {
        if (i >= _size)
        {
            LL::error_print("out of bounds access on const VertexArray!\n");
        }
        return VertexForm(_attributes, array + _attributes.elements() * i);
    }

    VertexForm VertexArray::operator[](std::size_t i)
    {
        if (i >= _size)
        {
            if (i >= _capacity)
            {
                LL::error_print("out of bounds VertexArray access; growing array...\n");
                reserve(2 * _capacity);
            }
            _size = i + 1;
        }
        return VertexForm(_attributes, array + _attributes.elements() * i);
    }
}
