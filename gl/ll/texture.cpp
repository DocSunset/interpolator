#include "texture.h"
#include "error.h"
#include <GLES3/gl3.h>

namespace
{
    using namespace GL::LL;
    GLenum target_to_gl(Texture::Target target)
    {
        switch (target)
        {
            case Texture::Target::TEX_2D:
                return GL_TEXTURE_2D;
            case Texture::Target::TEX_3D:
                return GL_TEXTURE_3D;
            case Texture::Target::TEX_2D_ARRAY:
                return GL_TEXTURE_2D_ARRAY;
            case Texture::Target::TEX_CUBE_MAP:
                return GL_TEXTURE_CUBE_MAP;
            default:
                error_print("unrecognized texture target\n");
                return GL_TEXTURE_2D;
        }
    }
    
    GLenum target_to_gl_binding(Texture::Target target)
    {
        switch (target)
        {
            case Texture::Target::TEX_2D:
                return GL_TEXTURE_BINDING_2D;
            case Texture::Target::TEX_3D:
                return GL_TEXTURE_BINDING_3D;
            case Texture::Target::TEX_2D_ARRAY:
                return GL_TEXTURE_BINDING_2D_ARRAY;
            case Texture::Target::TEX_CUBE_MAP:
                return GL_TEXTURE_BINDING_CUBE_MAP;
            default:
                error_print("unrecognized texture target\n");
                return GL_TEXTURE_BINDING_2D;
        }
    }
}

namespace GL::LL
{
    Texture::Texture(Target t)
        : target{target_to_gl(t)}
    {
        glGenTextures(1, &handle);
        if (any_error()) error_print("unexpected error after glGenTextures.\n");
    }

    Texture::~Texture()
    {
        glDeleteTextures(1, &handle);
        if (any_error()) error_print("unexpected error after glDeleteTextures.\n");
    }

    Texture::operator bool() const
    {
#ifdef DEBUG
        return glIsTexture(handle);
#else
        return handle != 0;
#endif
    }

    int Texture::get_active_texture_unit()
    {
        GLint i;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &i);
        return i - GL_TEXTURE0;
    }

    int Texture::max_combined_texture_image_units()
    {
        GLint i;
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &i);
        return i;
    }

    void Texture::set_active_texture_unit(unsigned int unit)
    {
        if (unit > max_combined_texture_image_units())
        {
            error_print("Error: texture unit greater than max combined units requested, ignoring\n");
            return;
        }
        glActiveTexture(GL_TEXTURE0 + unit);
    }

    GLuint Texture::current_binding(Texture::Target t)
    {
        GLint i;
        glGetIntegerv(target_to_gl_binding(t), &i);
        return i;
    }

    TextureBinding::TextureBinding(const Texture& texture, unsigned int unit)
        : t{texture}
    {
        Texture::set_active_texture_unit(unit);
        glBindTexture(t.target, t.handle);
        if (any_error()) error_print("unexpected error after glBindTexture.\n");
    }
}
