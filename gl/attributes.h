#pragma once

#include <string>
#include <vector>
#include <GLES3/gl3.h>

namespace GL
{
    class Attribute
    {
    private:
        friend class AttributeManifest;
        Attribute(GLuint program, GLuint index, char * name_buffer, GLint buffer_size);

        std::string _name;
    public:
        Attribute(const char * name, GLenum type, GLint size);
        const char * name() const {return _name.c_str();}
    };

    class AttributeManifest
    {
        std::vector<Attribute> attributes;
    public:
        AttributeManifest(GLuint program);
        ~AttributeManifest();

        bool has(const char * name) const;
        bool operator==(AttributeManifest& other);
    };
}
