#include "sampler.h"
#include "error.h"
#include <GLES3/gl3.h>

namespace
{
    using namespace GL::LL;

    GLenum wrap_axis_to_gl(Sampler::WrapAxis axis)
    {
        switch(axis)
        {
            case Sampler::WrapAxis::S:
                return GL_TEXTURE_WRAP_S;
            case Sampler::WrapAxis::R:
                return GL_TEXTURE_WRAP_R;
            case Sampler::WrapAxis::T:
                return GL_TEXTURE_WRAP_T;
            default:
                error_print("unexpected Sampler::WrapAxis\n");
                return GL_TEXTURE_WRAP_S;
        }
    }

    GLenum wrap_mode_to_gl(Sampler::WrapMode mode)
    {
        switch(mode)
        {
            case Sampler::WrapMode::CLAMP_TO_EDGE:
                return GL_CLAMP_TO_EDGE;
            case Sampler::WrapMode::REPEAT:
                return GL_REPEAT;
            case Sampler::WrapMode::MIRRORED_REPEAT:
                return GL_MIRRORED_REPEAT;
            default:
                error_print("unexpected Sampler::WrapMode\n");
                return GL_REPEAT;
        }
    }

    Sampler::WrapMode gl_to_wrap_mode(GLenum e)
    {
        switch(e)
        {
            case GL_CLAMP_TO_EDGE:
                return Sampler::WrapMode::CLAMP_TO_EDGE;
            case GL_REPEAT:
                return Sampler::WrapMode::REPEAT;
            case GL_MIRRORED_REPEAT:
                return Sampler::WrapMode::MIRRORED_REPEAT;
            default:
                error_print("invalid wrap mode gl enum\n");
                return Sampler::WrapMode::REPEAT;
        }
    }

    GLenum min_to_gl(Sampler::MinFilterMode mode)
    {
        switch(mode)
        {
            case Sampler::MinFilterMode::NEAREST:
                return GL_NEAREST;
            case Sampler::MinFilterMode::LINEAR:
                return GL_LINEAR;
            case Sampler::MinFilterMode::NEAREST_MIPMAP_LINEAR:
                return GL_NEAREST_MIPMAP_LINEAR;
            case Sampler::MinFilterMode::NEAREST_MIPMAP_NEAREST:
                return GL_NEAREST_MIPMAP_NEAREST;
            case Sampler::MinFilterMode::LINEAR_MIPMAP_NEAREST:
                return GL_LINEAR_MIPMAP_NEAREST;
            case Sampler::MinFilterMode::LINEAR_MIPMAP_LINEAR:
                return GL_LINEAR_MIPMAP_LINEAR;
            default:
                error_print("unexpected Sample::MinFilterMode\n");
                return GL_NEAREST_MIPMAP_LINEAR;
        }
    }

    Sampler::MinFilterMode gl_to_min(GLenum e)
    {
        switch(e)
        {
            case GL_NEAREST:
                return Sampler::MinFilterMode::NEAREST;
            case GL_LINEAR:
                return Sampler::MinFilterMode::LINEAR;
            case GL_NEAREST_MIPMAP_LINEAR:
                return Sampler::MinFilterMode::NEAREST_MIPMAP_LINEAR;
            case GL_NEAREST_MIPMAP_NEAREST:
                return Sampler::MinFilterMode::NEAREST_MIPMAP_NEAREST;
            case GL_LINEAR_MIPMAP_NEAREST:
                return Sampler::MinFilterMode::LINEAR_MIPMAP_NEAREST;
            case GL_LINEAR_MIPMAP_LINEAR:
                return Sampler::MinFilterMode::LINEAR_MIPMAP_LINEAR;
            default:
                error_print("invalid min filter gl enum\n");
                return Sampler::MinFilterMode::NEAREST_MIPMAP_LINEAR;
        }
    }

    GLenum mag_to_gl(Sampler::MagFilterMode mode)
    {
        switch(mode)
        {
            case Sampler::MagFilterMode::NEAREST:
                return GL_NEAREST;
            case Sampler::MagFilterMode::LINEAR:
                return GL_LINEAR;
            default:
                error_print("unexpected Sample::MagFilterMode\n");
                return GL_LINEAR;
        }
    }

    Sampler::MagFilterMode gl_to_mag(GLenum e)
    {
        switch(e)
        {
            case GL_NEAREST:
                return Sampler::MagFilterMode::NEAREST;
            case GL_LINEAR:
                return Sampler::MagFilterMode::LINEAR;
            default:
                error_print("invalid mag filter gl enum\n");
                return Sampler::MagFilterMode::LINEAR;
        }
    }
}

namespace GL::LL
{
    Sampler::Sampler()
    {
        glGenSamplers(1, &handle);
        if (any_error()) error_print("unexpected error after glGenSamplers.\n");
    }

    Sampler::operator bool() const
    {
#ifdef DEBUG
        return glIsSampler(handle);
#else
        return handle != 0;
#endif
    }

    GLuint Sampler::current_binding()
    {
        GLint i;
        glGetIntegerv(GL_SAMPLER_BINDING, &i);
        return i;
    }

    SamplerBinding::SamplerBinding(const Sampler& sampler, unsigned int unit)
        : s{sampler}
    {
        glBindSampler(unit, s.gl_handle());
        if (any_error()) error_print("unexpected error after glBindSampler.\n");
    }

    void SamplerBinding::set_wrap(Sampler::WrapAxis axis, Sampler::WrapMode mode)
    {
        glSamplerParameteri(s.gl_handle(), wrap_axis_to_gl(axis), wrap_mode_to_gl(mode));
        if (any_error()) error_print("unexpected error after glSamplerParameter in set_wrap\n");
    }

    Sampler::WrapMode SamplerBinding::get_wrap(Sampler::WrapAxis axis)
    {
        GLint i;
        glGetSamplerParameteriv(s.gl_handle(), wrap_axis_to_gl(axis), &i);
        return gl_to_wrap_mode(i);
    }

    void SamplerBinding::set_min_filter(Sampler::MinFilterMode mode)
    {
        glSamplerParameteri(s.gl_handle(), GL_TEXTURE_MIN_FILTER, min_to_gl(mode));
        if (any_error()) error_print("unexpected error after glSamplerParameter in set_min_filter\n");
    }

    Sampler::MinFilterMode SamplerBinding::get_min_filter()
    {
        GLint i;
        glGetSamplerParameteriv(s.gl_handle(), GL_TEXTURE_MIN_FILTER, &i);
        return gl_to_min(i);
    }

    void SamplerBinding::set_mag_filter(Sampler::MagFilterMode mode)
    {
        glSamplerParameteri(s.gl_handle(), GL_TEXTURE_MAG_FILTER, mag_to_gl(mode));
        if (any_error()) error_print("unexpected error after glSamplerParameter in set_mag_filter\n");
    }

    Sampler::MagFilterMode SamplerBinding::get_mag_filter()
    {
        GLint i;
        glGetSamplerParameteriv(s.gl_handle(), GL_TEXTURE_MAG_FILTER, &i);
        return gl_to_mag(i);
    }
}
