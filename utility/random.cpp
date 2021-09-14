#include "random.h"
#include <random>

using Generator = std::ranlux24;
using Distribution = std::uniform_real_distribution<float>;
using Seed = std::random_device;

namespace Utility
{
    float nrandf()
    {
        static thread_local Generator gen{Seed{}()};
        static thread_local Distribution dist(0.0f, 1.0f);
        return dist(gen);
    }

    float brandf()
    {
        static thread_local Generator gen{Seed{}()};
        static thread_local Distribution dist(-1.0f, 1.0f);
        return dist(gen);
    }

    float rrandf(float min, float max)
    {
        static thread_local Generator gen{Seed{}()};
        if (max < min) std::swap(min, max);
        Distribution dist(min, max);
        return dist(gen);
    }
}
