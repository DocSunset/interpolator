#pragma once

namespace Component
{
    struct FMSynthParameters
    {
        float frequency_hz;
        float amplitude;
        float feedback;
        float sampling_rate;
        static FMSynthParameters Random(float sampling_rate);
    };

    struct FMSynth
    {
        float _phase_rads;
        float _last_out[2];
        FMSynthParameters p;

        void init()
        {
            _phase_rads = 0;
            _last_out[0] = 0;
            _last_out[1] = 0;
        }

        float tick();
    };

    void fm_synth_audio_callback(void* userdata, unsigned char * stream, int bytes);
}
