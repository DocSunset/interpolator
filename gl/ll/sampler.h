#pragma once
#include <GLES3/gl3.h>

namespace GL::LL
{
    class Sampler
    {
    public:
        enum class WrapAxis
        {
            S,
            R,
            T,
        };

        enum class WrapMode
        {
            CLAMP_TO_EDGE,
            REPEAT,
            MIRRORED_REPEAT,
        };

        enum class MinFilterMode
        {
            NEAREST,
            LINEAR,
            NEAREST_MIPMAP_NEAREST,
            LINEAR_MIPMAP_NEAREST,
            NEAREST_MIPMAP_LINEAR,
            LINEAR_MIPMAP_LINEAR,
        };

        enum class MagFilterMode
        {
            NEAREST,
            LINEAR,
        };

    private:
        GLuint handle;

    public:
        GLuint gl_handle() const {return handle;}
        Sampler();
        operator bool() const;

        static GLuint current_binding();
    };

    class SamplerBinding
    {
        const Sampler& s;
    public:
        SamplerBinding(const Sampler&, unsigned int image_unit);
        void set_wrap(Sampler::WrapAxis, Sampler::WrapMode);
        Sampler::WrapMode get_wrap(Sampler::WrapAxis);
        void set_min_filter(Sampler::MinFilterMode);
        Sampler::MinFilterMode get_min_filter();
        void set_mag_filter(Sampler::MagFilterMode);
        Sampler::MagFilterMode get_mag_filter();
        // todo: LOD, compare mode, compare func
    };

    inline SamplerBinding bind(const Sampler& s, unsigned int u) {return SamplerBinding(s, u);}
}
