#include "fmsynth.h"
#include <cmath>
#include "utility/random.h"
#include "utility/mtof.h"

namespace Component
{
    FMSynthParameters FMSynthParameters::Random(float sampling_rate)
    {
        using namespace Utility;
        return {mtof(rrandf(0, 127)), 0.0, rrandf(0, 1), sampling_rate};
    }

    // It is ill advised to do math with parameter sets using different
    // sampling rates. Care must be taken.
    FMSynthParameters operator+(const FMSynthParameters& lhs, const FMSynthParameters& rhs)
    {
        return { lhs.frequency_hz + rhs.frequency_hz
               , lhs.amplitude + rhs.amplitude
               , lhs.feedback + rhs.feedback
               , lhs.sampling_rate
               };
    }

    FMSynthParameters operator*(float scalar, const FMSynthParameters& vector)
    {
        return { scalar * vector.frequency_hz
               , scalar * vector.amplitude
               , scalar * vector.feedback
               , vector.sampling_rate
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

    float FMSynth::tick()
    {
        constexpr float pi = 3.141592653589793238462643383279502884197169399375105820974944;
        constexpr float twopi = 2.0 * pi;
        constexpr float lotspi = twopi * 64;

        // smooth out parameter changes
        s = 0.1 * p + 0.9 * s; // s.sampling_rate == p.sampling_rate

        float out;
        float phase_incr = twopi * s.frequency_hz / s.sampling_rate; 
        float feedback = s.feedback * twopi * (_last_out[0] + _last_out[1]); 
        _phase_rads += phase_incr;
        // modulo to ensure rollover on multiple of two pi and without loss of precision
        _phase_rads = std::fmod(_phase_rads, lotspi);
        out = s.amplitude * std::cos(_phase_rads + feedback);
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
