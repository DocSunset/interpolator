#pragma once

#include <GLES3/gl3.h>

namespace GL::LL
{
    class Texture
    {
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
