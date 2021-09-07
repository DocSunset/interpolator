#include "vertex_form.h"

namespace GL
{
    VertexForm::VertexForm(const LL::AttributeManifest& a)
        : attributes{a}
    {
        data_view = malloc(attributes.bytes());
    }

    bool VertexForm::has(const char * name) const
    {
        return attributes.has(name);
    }

    Attribute VertexForm::operator[](const char * name)
    {
        return Attribute(*this);
    }

    Attribute::Attribute(VertexForm& f)
        : form{f}
    {
    }

    Attribute Attribute::operator[](std::size_t i)
    {
        return Attribute(form);
    }

    float Attribute::as_float()
    {
        return 0.5;
    }
}
