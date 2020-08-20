#include <cmath>
#include <random>
#include <chrono>
#include <iostream>
#include <vector>
#include "include/fire-hpp/fire.hpp"
#include "include/bitmap/bitmap_image.hpp"
#include "../interpolator/marier_spheres.h"
#include <Eigen/Core>

using Scalar = float;
using ID = unsigned int;
using Vec2 = Eigen::Vector2f;
using RGBVec = Eigen::Vector3f;
using Interpolator = MarierSpheresInterpolator<Scalar, ID, Vec2, RGBVec>;
using Demo = typename Interpolator::Demo;

Interpolator interpolator;
std::vector<Demo> demos;

int fired_main(
        unsigned int x = fire::arg("x", "The horizontal dimension in pixels", 500), 
        unsigned int y = fire::arg("y", "The vertical dimension in pixels", 500), 
        unsigned int n = fire::arg("n", "The number of demonstrations", 5))
{
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
            auto out = interpolator.query(q, demos);
            img.set_pixel(xpix, ypix,
                    (unsigned char)std::round(out.x() * 255),
                    (unsigned char)std::round(out.y() * 255),
                    (unsigned char)std::round(out.z() * 255)); 
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
        const RGBVec& c = demo.p;
        draw.pen_color(0,0,0);
        draw.pen_width(1);
        draw.circle(v.x() * x, v.y() * y, 7);
        draw.pen_color(255,255,255);
        draw.circle(v.x() * x, v.y() * y, 5);
        draw.pen_color(c.x() * 255, c.y() * 255, c.z() * 255);
        draw.pen_width(3);
        draw.circle(v.x() * x, v.y() * y, 2);
    }

    img.save_image("interpolated_colors.bmp");

    return 0;
}

FIRE(fired_main)