#pragma once

#include "attributes.h"
#include <GLES3/gl3.h>
#include <memory>

namespace GL::LL
{
    class Program;

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

        void insert(const Attribute& a);
        void insert(Attribute&& a);

        const Attribute& operator[](GLuint i);
              Attribute& operator[](GLuint i) const;

        bool has(const Attribute& a) const;

        GLuint size() const;
    };
}
