#pragma once

#include "ll.h"

namespace GL
{
    /* The idea with the vertex form is that the user calls `with` to select
     * an attribute in the form to modify, then `set` returns a mutated form
     * with the requested value. Ideally, I should like the form semantics to
     * be logically immutable; you can never actually modify a form, just get
     * modified forms from it.
     *
     * Ideally, modifying the form should be equivalent to just dumping data
     * into a heterogeneous buffer (e.g. a memory mapped GL array buffer). This
     * is at odds with the idea that the form should be immutable; the memory
     * has to live somewhere, and it has to be modified, and it is not
     * efficient to later have to copy the final modified form to the desired
     * location anyways.
     *
     * For testing purposes I think it makes sense for there to be a standalone
     * form object that owns its own memory. But I don't think that it would be
     * useful to actually make one in practice; the user will get and set forms
     * that point directly into a vertex array's memory.
     */
    class Attribute;

    class VertexForm
    {
        const LL::AttributeManifest& attributes;
        LL::AttributeElement * data_view;
    public:
        VertexForm(const LL::AttributeManifest& attr);
        VertexForm(const LL::AttributeManifest& attr, void * data);

        const Attribute operator[](const char * name) const;
              Attribute operator[](const char * name);

        bool has(const char * name) const;
        std::size_t stride() const;
    };

    class Attribute
    {
    private:
        VertexForm& form;
        LL::AttributeElement * data_view;
        LL::AttributeType type;
    public:
        // pointer to a particular attribute in a form
        Attribute(VertexForm& form, LL::AttributeElement * data_view, LL::AttributeType type);
        Attribute operator[](std::size_t i);
        GLfloat as_float(std::size_t i = 0);
        Attribute step(std::size_t i = 1);
        std::size_t size() const;

        template<std::size_t pos = 0, typename T, typename ... Ts>
        VertexForm& set(T t, Ts ... ts)
        {
            if (pos >= size()) return form;
            set(pos, t);
            return set<pos+1>(ts ...);
        }

        template<std::size_t pos>
        VertexForm& set()
        {
            return form;
        }

        VertexForm& set(std::size_t pos, GLfloat f);
        VertexForm& set(std::size_t pos, GLint i);
        VertexForm& set(std::size_t pos, GLuint u);
    };
}
