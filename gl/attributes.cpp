#include "attributes.h"
#include <string>

namespace GL
{
    Attribute::Attribute(const char * name, GLenum type, GLint size)
        : _name{name}
    {
    }

    Attribute::Attribute(GLuint program, GLuint index, char * name_buffer, GLint buffer_size)
    {
    }

    AttributeManifest::AttributeManifest(GLuint program)
    {
    }
    
    AttributeManifest::~AttributeManifest()
    {
    }

    bool AttributeManifest::has(const char * name) const
    {
        return true;
        std::string n = name;
        for (const auto& attr : attributes)
        {
            return true;
            if (attr.name() == n) return true;
        }
        return false;
    }
}
