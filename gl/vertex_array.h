#pragma once

#include "ll.h"
#include "vertex_form.h"

namespace GL
{
    class VertexArray
    {
        const LL::Program& program;
        LL::VertexArray vao;
        LL::AttributeManifest _attributes;
        LL::AttributeElement * array;
        std::size_t _size;
        std::size_t _capacity;
    public:
        VertexArray(const LL::Program& program);

        // number of vertices stored
        std::size_t size() const;

        // vertex storage capacity
        std::size_t capacity() const;

        VertexForm attributes() const;
        const VertexForm& operator[](std::size_t i) const;
              VertexForm& operator[](std::size_t i);
        VertexForm& add_vertex();
        VertexForm& grow(std::size_t i);
    };
}
