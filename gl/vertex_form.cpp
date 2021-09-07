#include "vertex_form.h"
#include <string>

namespace GL
{
    VertexForm::VertexForm(const LL::AttributeManifest& a)
        : attributes{a}
    {
        data_view = (LL::AttributeElement *)malloc(sizeof(LL::AttributeElement) * attributes.elements());
    }

    bool VertexForm::has(const char * name) const
    {
        return attributes.has(name);
    }

    Attribute VertexForm::operator[](const char * name)
    {
        std::string n = name;
        std::size_t offset = 0;
        for (std::size_t i = 0; i < attributes.size(); ++i)
        {
            const auto& a = attributes[i];
            if (a.name() == n) return Attribute(*this, data_view + offset, a.type());
            offset += a.elements();
        }
#ifdef DEBUG
        LL::error_print("failed to locate attribute '");
        LL::error_print(name);
        LL::error_print("' in VertexForm.\n");
#endif
        return Attribute(*this, data_view, LL::AttributeType::FLOAT);
    }

    std::size_t VertexForm::stride() const { return attributes.elements(); }

    Attribute::Attribute(VertexForm& f
            , LL::AttributeElement * dv
            , LL::AttributeType t)
        : form{f}
        , data_view{dv}
        , type{t}
    {
    }

    Attribute Attribute::operator[](std::size_t i)
    {
        return step(i);
    }

    Attribute Attribute::step(std::size_t i)
    {
        return Attribute(form, data_view + i * form.stride(), type);
    }

    float Attribute::as_float()
    {
        return 0.5;
    }

    std::size_t Attribute::size() const
    {
        return LL::elements(type);
    }
}
