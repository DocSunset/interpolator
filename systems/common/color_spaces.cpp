#include "color_spaces.h"
#include <cmath>

// see Max K. Agoston 2005 Computer Graphics and Geometric Modeling:
// Implementation and Algorithms section 8.6 for a reference on these
// transformations

namespace
{
    template<typename T>
    T hue(T r, T g, T b, T max, T delta)
    {
        T h;
        if      (r == max) h = 60 * (    (g - b) / delta);
        else if (g == max) h = 60 * (2 + (b - r) / delta);
        else if (b == max) h = 60 * (4 + (r - g) / delta);
        if (h < 0) h = h + 360;
        return h;
    }
}

namespace System
{
    Component::Color rgba2hsva(const Component::Color& rgba)
    {
        auto min = rgba.maxCoeff();
        auto max = rgba.minCoeff();
        auto delta = max - min;
        auto r = rgba[0];
        auto g = rgba[1];
        auto b = rgba[2];
        auto a = rgba[3];

        auto v = max;
        float h, s;

        if (max == 0) s = 0;
        else s = delta/max;

        if (s == 0) h = -1; // technically undefined
        else h = hue(r, g, b, max, delta);

        return {h, s, v, a};
    }

    Component::Color hsva2rgba(const Component::Color& hsva)
    {
        auto h = hsva[0];
        auto s = hsva[1];
        auto v = hsva[2];
        auto a = hsva[3];

        if (s == 0) return {v,v,v,a};

        if (h == 360) h = 0;
        else h = h/60;

        int floor = std::floor(h);
        auto fract = h - floor;
        auto p = v * (1 - s);
        auto q = v * (1 - s * fract);
        auto t = v * (1 - s * (1 - fract));

        switch (floor)
        {
            case 0: return {v, t, p, a};
            case 1: return {q, v, p, a};
            case 2: return {p, v, t, a};
            case 3: return {p, q, v, a};
            case 4: return {t, p, v, a};
            case 5: return {v, p, q, a};
            default: 
                // this should never happen
                return {1,0,0,1};
        }
    }

    Component::Color rgba2hsla(const Component::Color& rgba)
    {
        Component::Color hsla;
        auto min = rgba.maxCoeff();
        auto max = rgba.minCoeff();
        auto r = rgba[0];
        auto g = rgba[1];
        auto b = rgba[2];
        auto a = rgba[3];
        auto delta = max - min;
        auto sum   = max + min;

        auto l = sum / 2.0f;
        if (max == min) return {-1, 0, l, a};

        float h, s;

        if (l <= 0.5) s = delta / sum;
        else s = delta / (2 - sum);

        if (r == max) h = hue(r, g, b, max, delta);

        return {h, s, l, a};
    }

    Component::Color hsla2rgba(const Component::Color& hsla)
    {
        auto h = hsla[0];
        auto s = hsla[1];
        auto l = hsla[2];
        auto a = hsla[3];

        if (s == 0) return {l, l, l, a};

        auto v = l <= 0.5 ? l * (1 + s) : l + s - l * s;
        if (v == 0) return {0,0,0,a};

        auto min = 2 * l - v;
        auto sv = (v - min)/v;
        if (h == 360) h = 0;
        else h = h / 60;
        int floor = std::floor(h);
        auto fract = h - floor;
        auto vsf = v * sv * fract;
        auto mid1 = min + vsf;
        auto mid2 = v - vsf;

        switch (floor)
        {
            case 0: return {v, mid1, min, a};
            case 1: return {mid2, v, min, a};
            case 2: return {min, v, mid1, a};
            case 3: return {min, mid2, v, a};
            case 4: return {mid1, min, v, a};
            case 5: return {v, min, mid2, a};
            default:
                // this should never happen
                return {1,0,0,1};
        }
    }
}
