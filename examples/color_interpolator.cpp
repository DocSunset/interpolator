#include <cmath>
#include <random>
#include <chrono>
#include <iostream>
#include <vector>
#include "../include/fire-hpp/fire.hpp"
#include "../include/bitmap/bitmap_image.hpp"
#include "../include/marier_spheres.h"

template<typename Scalar> struct Vec2
{
    Scalar x;
    Scalar y;
};

template<typename Scalar>
Vec2<Scalar> operator+(const Vec2<Scalar>& a, const Vec2<Scalar>& b)
{
    return {a.x + b.x, a.y + b.y};
}

template<typename Scalar>
Vec2<Scalar> operator-(const Vec2<Scalar>& a, const Vec2<Scalar>& b)
{
    return {a.x - b.x, a.y - b.y};
}

template<typename Scalar>
Vec2<Scalar>& operator+=(Vec2<Scalar>& a, const Vec2<Scalar>& b)
{
    return a = a + b;
}

template<typename Scalar>
Vec2<Scalar>& operator-=(Vec2<Scalar>& a, const Vec2<Scalar>& b)
{
    return a = a - b;
}

template<typename Scalar>
Vec2<Scalar> operator*(const Scalar& w, const Vec2<Scalar>& v)
{
    return {v.x * w, v.y * w};
}

template<typename Scalar>
Vec2<Scalar> operator*(const Vec2<Scalar>& v, const Scalar& w)
{
    return w * v;
}

template<typename Scalar>
Scalar norm(const Vec2<Scalar>& a)
{
    return std::sqrt(a.x * a.x + a.y * a.y);
}

template<typename Scalar> struct RGBVec
{
    Scalar red;
    Scalar green;
    Scalar blue;

    Scalar max() const {return std::max({red, green, blue});}
};

template<typename Scalar>
RGBVec<Scalar> operator+(const RGBVec<Scalar>& a, const RGBVec<Scalar>& b)
{
    return {a.red + b.red, a.green + b.green, a.blue + b.blue};
}

template<typename Scalar>
RGBVec<Scalar> operator-(const RGBVec<Scalar>& a, const RGBVec<Scalar>& b)
{
    return {a.red - b.red, a.green - b.green, a.blue - b.blue};
}

template<typename Scalar>
RGBVec<Scalar>& operator+=(RGBVec<Scalar>& a, const RGBVec<Scalar>& b)
{
    return a = a + b;
}

template<typename Scalar>
RGBVec<Scalar>& operator-=(RGBVec<Scalar>& a, const RGBVec<Scalar>& b)
{
    return a = a - b;
}

template<typename Scalar>
RGBVec<Scalar> operator*(const Scalar& w, const RGBVec<Scalar>& c)
{
    return {c.red * w, c.green * w, c.blue * w};
}

template<typename Scalar>
RGBVec<Scalar> operator*(const RGBVec<Scalar>& c, const Scalar& w)
{
    return w * c;
}

int fired_main(
        unsigned int x = fire::arg("x", "The horizontal dimension in pixels", 500), 
        unsigned int y = fire::arg("y", "The vertical dimension in pixels", 500), 
        unsigned int n = fire::arg("n", "The number of demonstrations", 25))
{
    using Scalar = float;
    using ID = unsigned int;
    using Vec2 = Vec2<Scalar>;
    using RGBVec = RGBVec<Scalar>;
    using Interpolator = MarierSpheresInterpolator<Scalar, ID, Vec2, RGBVec>;
    using Demo = typename Interpolator::Demo;
    Interpolator interpolator;
    std::vector<Demo> demos;

    bitmap_image img(x, y); 
    img.clear();

    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator (seed);
    std::uniform_real_distribution<Scalar> random(0, 1);
    while(n-- > 0)
    {
        auto v = Vec2{random(generator), random(generator)};
        auto c = RGBVec{random(generator), random(generator), random(generator)};
        Demo d{n, v, c};
        demos.push_back(d);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    for (unsigned int xpix = 0; xpix < x; ++xpix)
    {
        for (unsigned int ypix = 0; ypix < y; ++ypix)
        {
            auto q = Vec2{xpix/(Scalar)x, ypix/(Scalar)y};
            auto out = interpolator.query(q, demos) * (Scalar)255;
            img.set_pixel(xpix, ypix,
                    (unsigned char)std::round(out.red),
                    (unsigned char)std::round(out.green),
                    (unsigned char)std::round(out.blue)); 
        }
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto usec = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
    std::cout << "Generated " << x * y << " interpolations in " << usec << " microseconds\n" 
            << "About " << 1000000 * x * y / usec << " interpolations per second" 
            << std::endl;
    
    image_drawer draw(img);
    draw.pen_width(1);
    for (const auto& demo : demos)
    {
        const Vec2& v = demo.s;
        draw.pen_color(0,0,0);
        draw.circle(v.x * x, v.y * y, 5);
        draw.pen_color(255,255,255);
        draw.circle(v.x * x, v.y * y, 3);
    }

    img.save_image("interpolated_colors.bmp");

    return 0;
}

FIRE(fired_main)