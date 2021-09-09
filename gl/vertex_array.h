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
        std::size_t _size;
        std::size_t _capacity;
        LL::AttributeElement * array;
    public:
        VertexArray(const LL::Program& program);

        // number of vertices stored
        std::size_t size() const;

        // vertex storage capacity
        std::size_t capacity() const;

        std::size_t stride() const {return _attributes.elements();}

        const VertexForm operator[](std::size_t i) const;
              VertexForm operator[](std::size_t i);
        VertexForm add_vertex();
        void reserve(std::size_t i);

        const LL::AttributeElement * raw() const {return array;}
        LL::AttributeElement * raw() {return array;}
    };
}
