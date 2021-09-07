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
        template<typename ... Ts>
        VertexForm& set(Ts ... ts) {return form;}
        Attribute operator[](std::size_t i);
        float as_float();
        Attribute step(std::size_t i = 1);
        std::size_t size() const;
    };
}
