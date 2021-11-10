#include "texture.h"
#include "error.h"

namespace GL::LL
{
    Texture::Texture()
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
        return true;
#ifdef DEBUG
        return glIsTexture(handle);
#else
        return handle != 0;
#endif
    }

    TextureBinding::TextureBinding(const Texture& texture)
        : t{texture}
    {
        glBindTexture(t.target, t.handle);
    }

}
