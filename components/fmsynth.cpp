#include "fmsynth.h"
#include <cmath>
#include <iostream>
#include "utility/random.h"
#include "utility/mtof.h"
#include "simple/boundaries.h"
#include "simple/constants/pi.h"

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

    FMSynthParameters operator-(const FMSynthParameters& lhs, const FMSynthParameters& rhs)
    {
        return { lhs.parameters[0] - rhs.parameters[0]
               , lhs.parameters[1] - rhs.parameters[1]
               , lhs.parameters[2] - rhs.parameters[2]
               };
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
        // smooth out parameter changes
        s = 0.001 * p + 0.999 * s;

        // enforce parameter boundaries
        for (float& x : s.parameters) x = Simple::clip(x);

        phasor.frequency.set_midi(frequency_midi(s));
        _last_out[1] = _last_out[0];
        _last_out[0] = feedback(s) * Simple::pi * (_last_out[0] + _last_out[1]);
        _last_out[0] = amplitude(s) * std::cos(phasor.tick().radians() + _last_out[0]);
        return _last_out[0];
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
