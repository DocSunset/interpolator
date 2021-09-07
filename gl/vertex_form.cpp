#include "vertex_form.h"

namespace GL
{
    VertexForm::VertexForm(const LL::AttributeManifest& a)
        : attributes{a}
    {
        data_view = (LL::AttributeElement *)malloc(attributes.bytes());
    }

    bool VertexForm::has(const char * name) const
    {
        return attributes.has(name);
    }

    Attribute VertexForm::operator[](const char * name)
    {
        //auto a = attributes[name];
        return Attribute(*this, data_view, 0, 0, 0);
    }

    Attribute::Attribute(VertexForm& f
            , LL::AttributeElement * dv
            , std::size_t st , std::size_t sz , std::size_t bs)
        : form{f}
        , data_view{dv}
        , stride{st}
        , size{sz}
        , bytes{bs}
    {
    }

    Attribute Attribute::operator[](std::size_t i)
    {
        return step(i);
    }

    Attribute Attribute::step(std::size_t i)
    {
        return Attribute(form, data_view + i * stride, stride, size, bytes);
    }

    float Attribute::as_float()
    {
        return 0.5;
    }
}
