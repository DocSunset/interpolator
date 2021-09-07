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
        if (i == 0) return *this;
        return step(i);
    }

    Attribute Attribute::step(std::size_t i)
    {
        return Attribute(form, data_view + i * form.stride(), type);
    }

    GLfloat Attribute::as_float(std::size_t i)
    {
        using namespace ::GL::LL;
        if (i >= elements(type)) i = elements(type) - 1;
        auto view = data_view + i;
        switch (element_type(type))
        {
            case AttributeElementType::FLOAT:
                return view->f;
            case AttributeElementType::INT:
                return static_cast<float>(view->i);
            case AttributeElementType::UINT:
                return static_cast<float>(view->u);
            default:
                error_print("Unrecognized AttributeElementType in Attribute::as_float.\n");
                return 0.0f;
        }
    }

    std::size_t Attribute::size() const
    {
        return LL::elements(type);
    }

    VertexForm& Attribute::set(std::size_t position, GLfloat f)
    {
        using namespace ::GL::LL;
        auto view = data_view + position;
        switch (element_type(type))
        {
            case AttributeElementType::FLOAT:
                view->f = f;
                break;
            case AttributeElementType::INT:
                view->i = static_cast<GLint>(f);
                break;
            case AttributeElementType::UINT:
                view->u = static_cast<GLuint>(f);
                break;
            default:
                error_print("Unrecognized AttributeElementType in Attribute::as_float.\n");
        }
        return form;
    }

    VertexForm& Attribute::set(std::size_t position, GLint i)
    {
        using namespace ::GL::LL;
        auto view = data_view + position;
        switch (element_type(type))
        {
            case AttributeElementType::FLOAT:
                view->f = static_cast<GLfloat>(i);
                break;
            case AttributeElementType::INT:
                view->i = i;
                break;
            case AttributeElementType::UINT:
                view->u = static_cast<GLuint>(i);
                break;
            default:
                error_print("Unrecognized AttributeElementType in Attribute::as_float.\n");
        }
        return form;
    }

    VertexForm& Attribute::set(std::size_t position, GLuint u)
    {
        using namespace ::GL::LL;
        auto view = data_view + position;
        switch (element_type(type))
        {
            case AttributeElementType::FLOAT:
                view->f = static_cast<GLfloat>(u);
                break;
            case AttributeElementType::INT:
                view->i = static_cast<GLint>(u);
                break;
            case AttributeElementType::UINT:
                view->u = u;
                break;
            default:
                error_print("Unrecognized AttributeElementType in Attribute::as_float.\n");
        }
        return form;
    }
}
