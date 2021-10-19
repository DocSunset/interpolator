#include "fmsynth.h"
#include <cmath>
#include <iostream>
#include "utility/random.h"
#include "utility/mtof.h"
#include "simple/boundaries.h"

namespace Component
{
    FMSynthParameters FMSynthParameters::Random()
    {
        using namespace Utility;
        return {rrandf(0, 1), rrandf(0, 1), rrandf(0, 1)};
    }

    FMSynthParameters FMSynthParameters::Zero()
    {
        return {0, 0, 0};
    }

    FMSynthParameters operator+(const FMSynthParameters& lhs, const FMSynthParameters& rhs)
    {
        return { lhs.parameters[0] + rhs.parameters[0]
               , lhs.parameters[1] + rhs.parameters[1]
               , lhs.parameters[2] + rhs.parameters[2]
               };
    }

    FMSynthParameters operator*(float scalar, const FMSynthParameters& vector)
    {
        return { scalar * vector.parameters[0]
               , scalar * vector.parameters[1]
               , scalar * vector.parameters[2]
               };
    }

    FMSynthParameters operator*(const FMSynthParameters& vector, float scalar)
    {
        return scalar * vector;
    }

    FMSynthParameters& FMSynthParameters::operator+=(const FMSynthParameters& other)
    {
        return *this = *this + other;
    }

    FMSynthParameters& FMSynthParameters::operator*=(float scalar)
    {
        return *this = scalar * *this;
    }

    float frequency_midi(const FMSynthParameters& p)
    {
        return 127 * p.parameters[0];
    }

    float frequency_hz(const FMSynthParameters& p)
    {
        return Utility::mtof(frequency_midi(p));
    }

    float amplitude(const FMSynthParameters& p)
    {
        return p.parameters[1];
    }

    float feedback(const FMSynthParameters& p)
    {
        return p.parameters[2];
    }

    FMSynthParameters& set_frequency_midi(FMSynthParameters& p, float m)
    {
        p.parameters[0] = m/127.0f;
        return p;
    }

    FMSynthParameters& set_frequency_hz(FMSynthParameters& p, float f)
    {
        p.parameters[0] = Utility::ftom(f);
        return p;
    }

    FMSynthParameters& set_amplitude(FMSynthParameters& p, float a)
    {
        p.parameters[1] = a;
        return p;
    }

    FMSynthParameters& set_feedback(FMSynthParameters& p, float fb)
    {
        p.parameters[2] = fb;
        return p;
    }


    float FMSynth::tick()
    {
        constexpr float pi = 3.141592653589793238462643383279502884197169399375105820974944;
        constexpr float twopi = 2.0 * pi;
        constexpr float lotspi = twopi * 64;

        // smooth out parameter changes
        s = 0.001 * p + 0.999 * s; // s.sampling_rate == p.sampling_rate

        for (float& x : s.parameters)
        {
            x = Simple::clip(x);
        }

        float out;
        phasor.frequency.set_midi(frequency_midi(s));
        float fb = feedback(s) * twopi * (_last_out[0] + _last_out[1]);
        // modulo to ensure rollover on multiple of two pi and without loss of precision
        //_phase_rads = std::fmod(_phase_rads, lotspi);
        out = amplitude(s) * std::cos(phasor.tick().radians() + fb);
        _last_out[1] = _last_out[0];
        _last_out[0] = out;
        return out;
    }

    void fm_synth_audio_callback(void* userdata, unsigned char * stream, int bytes)
    {
        auto fm_synth = (FMSynth*)userdata;
        float * fstream = (float*)stream;
        int samples = bytes / sizeof(float);

        for (int i = 0; i < samples; ++i)
        {
            fstream[i] = fm_synth->tick();
        }
    }
}
