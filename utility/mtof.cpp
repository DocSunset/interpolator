#include "mtof.h"
#include <cmath>

namespace Utility
{
    float mtof(float midinote)
    {
        return 440.0f * std::pow(2, (midinote - 69.0f) / 12.0f);
    }

    float ftom(float freq_hz)
    {
        return 12.0f * std::log(freq_hz / 440.0f) / std::log(2.0f) + 69.0f;
    }
}
