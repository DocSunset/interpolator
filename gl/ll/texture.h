#pragma once

#include <GLES3/gl3.h>

namespace GL::LL
{
    class Texture
    {
    public:
        enum class Target
        {
            TEX_2D,
            TEX_3D,
            TEX_2D_ARRAY,
            TEX_CUBE_MAP,
        };
    private:
        friend class TextureBinding;
        GLuint handle;
    public:
        Texture();
        ~Texture();

        operator bool() const;
    };

    class TextureBinding
    {
        const Texture& t;
    public:
        TextureBinding(const Texture& t);
    };

    inline TextureBinding bind(const Texture& t) {return TextureBinding(t);}
}
