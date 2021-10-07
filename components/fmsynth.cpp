#include "fmsynth.h"
#include <cmath>
#include "utility/random.h"
#include "utility/mtof.h"

namespace Component
{
    FMSynthParameters FMSynthParameters::Random(float sampling_rate)
    {
        using namespace Utility;
        return {mtof(rrandf(0, 127)), 0.5, rrandf(0, 1), sampling_rate};
    }

    float FMSynth::tick()
    {
        constexpr float pi = 3.141592653589793238462643383279502884197169399375105820974944;
        constexpr float twopi = 2.0 * pi;
        constexpr float lotspi = twopi * 64;
        float out;
        float phase_incr = twopi * p.frequency_hz / p.sampling_rate; 
        float feedback = p.feedback * twopi * (_last_out[0] + _last_out[1]); 
        _phase_rads += phase_incr;
        // modulo to ensure rollover on multiple of two pi and without loss of precision
        _phase_rads = std::fmod(_phase_rads, lotspi);
        out = p.amplitude * std::cos(_phase_rads + feedback);
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
