#pragma once
#include "simple/phasor.h"

namespace Component
{
    struct FMSynthParameters
    {
        float parameters[3];

        static FMSynthParameters Random();
        static FMSynthParameters Zero();

        FMSynthParameters& operator+=(const FMSynthParameters& other);
        FMSynthParameters& operator*=(float scalar);
    };

    float frequency_midi(const FMSynthParameters& p);
    float frequency_hz(const FMSynthParameters& p);
    float amplitude(const FMSynthParameters& p);
    float feedback(const FMSynthParameters& p);

    FMSynthParameters& set_frequency_midi(FMSynthParameters& p, float m);
    FMSynthParameters& set_frequency_hz(FMSynthParameters& p, float f);
    FMSynthParameters& set_amplitude(FMSynthParameters& p, float a);
    FMSynthParameters& set_feedback(FMSynthParameters& p, float fb);

    FMSynthParameters operator+(const FMSynthParameters& lhs, const FMSynthParameters& rhs);
    FMSynthParameters operator*(float lhs, const FMSynthParameters& rhs);
    FMSynthParameters operator*(const FMSynthParameters& lhs, float rhs);

    struct FMSynth
    {
        Simple::Phasor phasor;
        float _last_out[2];
        FMSynthParameters p, s;

        void init(float sr, FMSynthParameters p)
        {
            phasor.sampling_rate.set_hz(sr);
            phasor.frequency.set_midi(frequency_midi(p));
            phasor.set_cycles(0);
            _last_out[0] = 0;
            _last_out[1] = 0;
            s = p;
        }

        float tick();
    };

    void fm_synth_audio_callback(void* userdata, unsigned char * stream, int bytes);
}
