#include "attribute_manifest.h"
#include "program.h"
#include "error.h"
#include <vector>

namespace GL::LL
{
    struct AttributeManifest::Implementation
    {
        std::vector<Attribute> attributes;

        Implementation() {}
        Implementation(const Program& p)
        {
            GLint bufsize = p.max_attribute_name_length();
            GLint active_attributes = p.active_attributes();
            if (active_attributes == 0)
            {
                error_print("No active attributes in AttributeManifest.\n");
                return;
            }
            char * namebuf = (char *)malloc(bufsize);
            for (GLint i = 0; i < active_attributes; ++i)
            {
                attributes.push_back(Attribute(p.handle, i, namebuf, bufsize));
            }
            free(namebuf);
        }

        const Attribute& at(GLuint i) const {return attributes[i];}
              Attribute& at(GLuint i)       {return attributes[i];}

        const Attribute& at(const std::string& name) const
        {
            for (const auto& a : attributes)
                if (a._name == std::string(name)) return a;
            error_print("Could not find attribute \""); error_print(name.c_str()); error_print("\".\n");
            return attributes[0];
        }

        Attribute& at(const std::string& name)
        {
            for (auto& a : attributes)
                if (a._name == std::string(name)) return a;
            error_print("Could not find attribute \""); error_print(name.c_str()); error_print("\".\n");
            return attributes[0];
        }

        GLint size() const {return static_cast<GLint>(attributes.size());}

        bool operator==(const Implementation& other) const
        {
            if (attributes.size() != other.attributes.size()) return false;
            for (GLint i = 0; i < attributes.size(); ++i)
            {
                if (attributes[i] != other.attributes[i]) return false;
            }
            return true;
        }

        bool has(const Attribute& a) const
        {
            for (auto& b : attributes)
                if (a == b) return true;
            return false;
        }

        bool has(const char * name) const
        {
            std::string n = name;
            for (auto& a : attributes)
                if (a._name == n) return true;
            return false;
        }

        void add_attribute(const char * name, AttributeType type)
        {
            attributes.emplace_back(name, type);
        }

        std::size_t bytes() const
        {
            std::size_t b = 0;
            for (auto& a : attributes)
            {
                b += a.bytes();
            }
            return b;
        }
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

    bool AttributeManifest::operator==(const AttributeManifest& other) const
    {
        return *pimpl == *other.pimpl;
    }

    bool AttributeManifest::operator!=(const AttributeManifest& other) const
    {
        return not (*pimpl == *other.pimpl);
    }

    const Attribute& AttributeManifest::operator[](GLuint i)       { return pimpl->at(i); }
          Attribute& AttributeManifest::operator[](GLuint i) const { return pimpl->at(i); }

    const Attribute& AttributeManifest::operator[](const std::string& name)       { return pimpl->at(name); }
          Attribute& AttributeManifest::operator[](const std::string& name) const { return pimpl->at(name); }

    bool AttributeManifest::has(const Attribute& a) const { return pimpl->has(a); }

    bool AttributeManifest::has(const char * name) const { return pimpl->has(name); }

    GLint AttributeManifest::size() const { return pimpl->size(); }

    void AttributeManifest::add_attribute(const char * name, AttributeType type)
    {
        pimpl->add_attribute(name, type);
    }

    std::size_t AttributeManifest::bytes() const { return pimpl->bytes(); }
}
