#include <cmath>
#include <random>
#include <string>
#include <chrono>
#include <iostreams>
#include "include/fire-hpp/fire.hpp"
#include "include/bitmap/bitmap_image.hpp"

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
    a = a + b;
}

template<typename Scalar>
Vec2<Scalar>& operator-=(Vec2<Scalar>& a, const Vec2<Scalar>& b)
{
    a = a - b;
}

template<typename Scalar>
Vec2<Scalar> operator*(const Scalar& w, const Vec2<Scalar>& v)
{
    return {v.x * w, v.y * w};
}

template<typename Scalar>
Vec2<Scalar> operator*(const Vec2<Scalar>& v, const Scalar& w)
{
    return w * c;
}

template<typename Scalar>
Scalar norm(const Vec2<Scalar>& a, const Vec2<Scalar>& b)
{
    Vec2<Scalar> difference = a - b;
    return std::sqrt(a.x * a.x + a.y * a.y);
}

template<typename Scalar> struct RGBVec
{
    Scalar red;
    Scalar green;
    Scalar blue;

    Scalar max() const {return std::max({red, green, blue});}
}

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
    a = a + b;
}

template<typename Scalar>
RGBVec<Scalar>& operator-=(RGBVec<Scalar>& a, const RGBVec<Scalar>& b)
{
    a = a - b;
}

template<typename Scalar>
RGBVec<Scalar> operator*(const Scalar& w, const RGBVec<Scalar>& c)
{
    return {a.red * w, a.green * w, a.blue * w};
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
    using Demo = Demonstration<Scalar, Vec2<Scalar>, RGBVec<Scalar>>
    MarierSpheresInterpolator<Scalar, Vec2<Scalar>, RGBVec<Scalar>> interpolator();

    bitmap_image img(x, y); 
    img.clear();

    std::uniform_real_distribution<Scalar> random(0, 1);
    auto start = std::chrono::high_resolution_clock::now();
    while(n-- > 0)
    {
        auto v = Vec2(random(), random());
        auto c = RGBVec(random(), random(), random());
        Demo d{std::to_string(n), v, c};
        interpolator.add_demo(d);
    }
    
    for (unsigned int xpix = 0; xpix < x; ++xpix)
    {
        for (unsigned int ypix = 0; ypix < y; ++ypix)
        {
            auto q = Vec2(xpix/x, ypix/y);
            auto out = interpolator.query(q) * 255;
            img.set_pixel(xpix, ypix, 
                    (unsigned char)std::round(out.red),
                    (unsigned char)std::round(out.green),
                    (unsigned char)std::round(out.blue));
        }
    }
    auto stop = std::chrono::high_resolution_clock::now();
    std::cout << "Generated " << x * y << "interpolations in " << 
            std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()
            << "microseconds";

    img.save_image("interpolated_colors.bmp");
}

FIRE(fired_main)