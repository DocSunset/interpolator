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

        const Attribute& operator[](GLuint i) const;
              Attribute& operator[](GLuint i);

        const Attribute& operator[](const std::string& name) const;
              Attribute& operator[](const std::string& name);

        GLint size() const;

        bool has(const Attribute& a) const;
        bool has(const char * name) const;

        void add_attribute(const char * name, AttributeType type);

        // number of elements in the manifest
        std::size_t elements() const;

        // size of manifest in bytes (== elements() * sizeof(AttributeElement))
        std::size_t bytes() const;

        // offset of given attribute in elements
        std::size_t offset_of(const char * name) const;
        std::size_t offset_of(std::size_t idx) const;

        // offset of given attribute in bytes
        std::size_t byte_offset_of(const char * name) const;
        std::size_t byte_offset_of(std::size_t idx) const;

        // index of given attribute in the manifest
        std::size_t index_of(const char * name) const;
    };
}
