#include "fmsynth.h"
#include <cmath>

namespace Component
{

    float FMSynth::tick()
    {
        constexpr float pi = 3.141592653589793238462643383279502884197169399375105820974944;
        constexpr float twopi = 2.0 * pi;
        float out;
        float phase_incr = twopi * p.frequency_hz / p.sampling_rate; 
        float feedback = p.feedback * pi * (_last_out[0] + _last_out[1]); 
        _phase_rads += phase_incr;
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
