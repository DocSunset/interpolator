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
        GLenum target;

    public:
        // return the raw GL handle
        // this is mainly provided for internal use and testing
        GLuint gl_handle() const {return handle;}

        Texture() = delete;
        Texture(Target t);
        ~Texture();

        operator bool() const;

        static int get_active_texture_unit();
        static int max_combined_texture_image_units();
        static void set_active_texture_unit(unsigned int);
        static GLuint current_binding(Target);
    };

    class TextureBinding
    {
        const Texture& t;
    public:
        TextureBinding(const Texture&, unsigned int image_unit);
    };

    inline TextureBinding bind(const Texture& t, unsigned int u) {return TextureBinding(t, u);}
}
