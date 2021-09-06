#include "attribute_manifest.h"
#include "program.h"
#include "error.h"

namespace GL::LL
{
    struct AttributeManifest::Implementation
    {
        Attribute _a = Attribute("foo", Attribute::Type::FLOAT);

        Implementation() {}
        Implementation(const Program& p)
        {
            if (not p) error_print("AttributeManifest constructed with invalid program.\n");
            char * namebuf = (char *)malloc(128);
            _a = Attribute(p.handle, 0, namebuf, 128);
            free(namebuf);
        }
        void insert(const Attribute& a) {}
        void insert(Attribute&& a) {}
        const Attribute& at(GLuint i) const {return _a;}
              Attribute& at(GLuint i)       {return _a;}
        bool has(const Attribute& a)
        {
            return a == _a;
        }
        GLuint size() const {return 0;}
    };

    AttributeManifest::AttributeManifest(const Program& p)
        : pimpl{std::make_unique<AttributeManifest::Implementation>(p)} { }
    AttributeManifest::AttributeManifest()
        : pimpl{std::make_unique<AttributeManifest::Implementation>()} { }

    AttributeManifest::~AttributeManifest() { }

    AttributeManifest::AttributeManifest(const AttributeManifest& copy)
        : pimpl{std::make_unique<AttributeManifest::Implementation>(*copy.pimpl)} { }

    AttributeManifest& AttributeManifest::operator=(const AttributeManifest& copy)
    {
        pimpl = std::make_unique<AttributeManifest::Implementation>(*copy.pimpl);
        return *this;
    }

    AttributeManifest::AttributeManifest(AttributeManifest&& move)
        : pimpl{std::move(move.pimpl)} { }

    AttributeManifest& AttributeManifest::operator=(AttributeManifest&& move)
    {
        pimpl = std::move(move.pimpl);
        return *this;
    }

    void AttributeManifest::insert(const Attribute& a) { pimpl->insert(a); }
    void AttributeManifest::insert(Attribute&& a)      { pimpl->insert(a); }

    const Attribute& AttributeManifest::operator[](GLuint i)       { return pimpl->at(i); }
          Attribute& AttributeManifest::operator[](GLuint i) const { return pimpl->at(i); }

    bool AttributeManifest::has(const Attribute& a) const { return pimpl->has(a); }
    GLuint AttributeManifest::size() const { return pimpl->size(); }
}
