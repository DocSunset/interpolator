#pragma once

#include "program.h"
#include "attributes.h"
#include <GLES3/gl3.h>
#include <memory>

namespace GL::LL
{
    class AttributeManifest
    {
        struct Implementation;
        std::unique_ptr<Implementation> pimpl;
    public:
        AttributeManifest();
        AttributeManifest(const Program& p);
        ~AttributeManifest();
        AttributeManifest(const AttributeManifest& copy);
        AttributeManifest& operator=(const AttributeManifest& copy);
        AttributeManifest(AttributeManifest&& move);
        AttributeManifest& operator=(AttributeManifest&& move);

        bool operator==(const AttributeManifest& other) const;
        bool operator!=(const AttributeManifest& other) const;

        const Attribute& operator[](GLuint i);
              Attribute& operator[](GLuint i) const;

        const Attribute& operator[](const std::string& name);
              Attribute& operator[](const std::string& name) const;

        GLint size() const;

        bool has(const Attribute& a) const;

        void add_attribute(const char * name, AttributeType type);
    };
}
