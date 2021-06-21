#include <cmath>
#include <cstdio>
#include <random>
#include <chrono>
#include <iostream>
#include <vector>
#include <tuple>
#include "include/fire-hpp/fire.hpp"
#include "include/bitmap/bitmap_image.hpp"
#include "../interpolator/marier_spheres.h"
#include <Eigen/Core>
#include <Eigen/LU>

using Scalar = float;
using ID = unsigned int;
using Vec2 = Eigen::Vector2f;
using RGBVec = Eigen::Vector3f;
using CIEXYZVec = Eigen::Vector3f;
using JzAzBzVec = Eigen::Vector3f;
using Interp = Interpolators<Scalar, ID, Vec2, JzAzBzVec>;

#define ANOTHER_INTERPOLATOR(type, ...) std::make_tuple(type{}, std::vector<type::Meta>{}, std::vector<type::Para>{}, type::Para{__VA_ARGS__}, bitmap_image{}, bitmap_image{})
auto interpolators = std::make_tuple
        ( ANOTHER_INTERPOLATOR(Interp::IntersectingNSpheres)
        , ANOTHER_INTERPOLATOR(Interp::InverseDistance, 4, 0.001, 0.0)
        );

RGBVec XYZ_to_RGB(const CIEXYZVec& xyz)
{
    RGBVec rgb;
    Eigen::Matrix3f a;
    a <<  3.24096994, -1.53738318, -0.49861076,
         -0.96924364,  1.8759675,   0.04155506,
          0.05563008, -0.20397696,  1.05697151;

    rgb = a * xyz; // apply linear transformation

    for (unsigned int i = 0; i < 3; i++) // apply gamme correction
    {
        auto u = rgb[i];
        u = u > 0.0031308 ? (1.055 * pow(u, 1 / 2.4) - 0.055) : 12.92 * u;
        u = u < 0.0 ? 0.0 : u;
        u = u > 1.0 ? 1.0 : u;
        rgb[i] = u;
    }

    return rgb;
}

CIEXYZVec RGB_to_XYZ(const RGBVec& rgb)
{
    CIEXYZVec xyz = rgb;
    Eigen::Matrix3f a;
    a <<  0.41239080,  0.35758434,  0.18048079,
          0.21263901,  0.71516868,  0.07219232,
          0.01933082,  0.11919478,  0.95053215;

    for (unsigned int i = 0; i < 3; i++) // reverse gamme correction
    {
        auto u = xyz[i];
        u = u > 0.04045 ? pow((u + 0.055) / 1.055, 2.4) : u / 12.92;
        xyz[i] = u;
    }

    return a * xyz; // reverse linear transformation
}

JzAzBzVec XYZ_to_JzAzBz(const CIEXYZVec& xyz)
{
    static const Eigen::Matrix3f M1 = (Eigen::Matrix3f() <<
        0.41478972,  0.579999,  0.0146480,
        -0.2015100,  1.120649,  0.0531008,
        -0.0166008,  0.264800,  0.6684799).finished();
    static const Eigen::Matrix3f M2 = (Eigen::Matrix3f() <<  
        0.5,         0.5,       0.0,
        3.524000,   -4.066708,  0.542708,
        0.199076,    1.096799, -1.295875).finished();
    constexpr float two5 = 1 << 5;
    constexpr float two7 = 1 << 7;
    constexpr float two12 = 1 << 12;
    constexpr float two14 = 1 << 14;
    constexpr float b = 1.15;
    constexpr float g = 0.66;
    constexpr float c1 = 3424.0 / two12;
    constexpr float c2 = 2413.0 / two7;
    constexpr float c3 = 2392.0 / two7;
    constexpr float n = 2610.0 / two14;
    constexpr float p = 1.7 * 2523.0 / two5;
    constexpr float d = -0.56;
    constexpr float d_0 = 1.6295499532821566e-11;
    auto x = xyz.x();
    auto y = xyz.y();
    auto z = xyz.z();

    // pre-adjust to improve iso-hue linearity (Safdar et al. eqn. 8
    auto x_ = b * x - (b - 1.0) * z;
    auto y_ = g * y - (g - 1.0) * x;
    CIEXYZVec xyz_(x_, y_, z);

    // transform xyz to cone primaries (Safdar et al. eqn. 9)
    Eigen::Vector3f lms = M1 * xyz_;

    // perceptual quantizer (Safdar et al. eqn. 10
    for (unsigned int i = 0; i < 3; ++i)
    {
        auto u = pow(lms[i] / 10000.0, n);
        lms[i] = pow( (c1 + c2 * u) / (1.0 + c3 * u), p );
    }

    // transform to correlates of opponent colour space (Safdar et al. eqn. 11)
    Eigen::Vector3f jab = M2 * lms;

    // improve wide-range lightness prediction (Safdar et al. eqn. 12)
    auto i = jab[0];
    jab[0] = ((1.0 + d) * i) / (1.0 + d * i) - d_0;

    return jab;
}

CIEXYZVec JzAzBz_to_XYZ(const JzAzBzVec& jab)
{
    static const Eigen::Matrix3f M1 = (Eigen::Matrix3f() <<
        0.41478972,  0.579999,  0.0146480,
        -0.2015100,  1.120649,  0.0531008,
        -0.0166008,  0.264800,  0.6684799).finished();
    static const Eigen::Matrix3f M2 = (Eigen::Matrix3f() <<  
        0.5,         0.5,       0.0,
        3.524000,   -4.066708,  0.542708,
        0.199076,    1.096799, -1.295875).finished();
    constexpr float two5 = 1 << 5;
    constexpr float two7 = 1 << 7;
    constexpr float two12 = 1 << 12;
    constexpr float two14 = 1 << 14;
    constexpr float b = 1.15;
    constexpr float g = 0.66;
    constexpr float c1 = 3424.0 / two12;
    constexpr float c2 = 2413.0 / two7;
    constexpr float c3 = 2392.0 / two7;
    constexpr float n = 2610.0 / two14;
    constexpr float p = 1.7 * 2523.0 / two5;
    constexpr float d = -0.56;
    constexpr float d_0 = 1.6295499532821566e-11;
    static const Eigen::Matrix3f M1inv = M1.inverse();
    static const Eigen::Matrix3f M2inv = M2.inverse();

    // eqn. 17
    auto iab = jab;
    auto j = jab[0] + d_0;
    iab[0] = j / (1.0 + d - d * j);

    // eqn. 18
    Eigen::Vector3f lms = M2inv * iab;

    // eqn. 19
    for (unsigned int i = 0; i < 3; ++i)
    {
        auto u = pow(lms[i], 1.0/p);
        lms[i] = 10000.0 * pow( (c1 - u) / (c3 * u - c2), 1.0/n );
    }

    // eqn. 20
    auto xyz_ = M1inv * lms;

    // eqn. 21 - 23
    auto x_ = xyz_[0];
    auto y_ = xyz_[1];
    auto z_ = xyz_[2];
    auto x = (x_ + (b - 1.0) * z_) / b;
    auto y = (y_ + (g - 1.0) * x) / g;
    auto z = z_;

    CIEXYZVec xyz{x, y, z};
    return xyz;
}

JzAzBzVec RGB_to_JzAzBz(const RGBVec& rgb)
{
    return XYZ_to_JzAzBz(RGB_to_XYZ(rgb));
}

RGBVec JzAzBz_to_RGB(const JzAzBzVec& jab)
{
    return XYZ_to_RGB(JzAzBz_to_XYZ(jab));
}

int fired_main
    ( unsigned int x = fire::arg("x", "The horizontal dimension in pixels", 500)
    , unsigned int y = fire::arg("y", "The vertical dimension in pixels", 500)
    , bool fast = fire::arg("f", "Set whether to calculate as though demonstrations are dynamic or not. Defaults to slow dynamic mode.")
    , unsigned int N = fire::arg("n", "The number of demonstrations", 5)
    , unsigned int C = fire::arg("c", "The number of contour lines for each preset in the contour map. Set to zero to disable the contour map", 10)
    )
{
    int i = 0;
    bool dynamic_demos = not fast;

    std::vector<Interp::Demo> demo;
    unsigned int n = N;
    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator (seed);
    std::uniform_real_distribution<Scalar> random(0, 1);
    while(n-- > 0)
    {
        auto v = Vec2{random(generator), random(generator)};
        auto c = RGB_to_JzAzBz(RGBVec{random(generator), random(generator), random(generator)});
        demo.push_back({n, v, c});
    }

    auto interpolate_and_draw = [&](auto& tup)
    {
        // unpack the tuple
        auto& interpolator = std::get<0>(tup);
        auto& meta = std::get<1>(tup);
        auto& para = std::get<2>(tup);
        auto& default_para = std::get<3>(tup);
        auto& colours_map = std::get<4>(tup);
        auto& contour_map = std::get<5>(tup);

        // resize the interpolator's lists to match the number of demonstrations
        meta.resize(demo.size());
        for (auto& m : meta) m = {};
        para.resize(demo.size());
        for (auto& p : para) p = default_para;

        // clear the bitmap images
        colours_map = bitmap_image{x, y};
        colours_map.clear();
        if (C)
        {
            contour_map = bitmap_image{x, y};
            contour_map.clear();
        }

            bool ran = false;
            auto start = std::chrono::high_resolution_clock::now();
            for (unsigned int xpix = 0; xpix < x; ++xpix)
            {
                for (unsigned int ypix = 0; ypix < y; ++ypix)
                {
                    auto q = Vec2{xpix/(Scalar)x, ypix/(Scalar)y};
                    JzAzBzVec interpolated_jab{0, 0, 0};
                    if constexpr (std::is_same_v<decltype(interpolator), Interp::IntersectingNSpheres>)
                    {
                        if (not dynamic_demos && not ran)
                        {
                            interpolator.dynamic_demos = true;
                            interpolator.query(q, demo, para, meta, interpolated_jab);
                            interpolator.dynamic_demos = false;
                            ran = true;
                        }
                        else interpolator.query(q, demo, para, meta, interpolated_jab);
                    }
                    else interpolator.query(q, demo, para, meta, interpolated_jab);
                    RGBVec out = JzAzBz_to_RGB(interpolated_jab);
                    colours_map.set_pixel(xpix, ypix,
                            (unsigned char)std::round(out.x() * 255),
                            (unsigned char)std::round(out.y() * 255),
                            (unsigned char)std::round(out.z() * 255));

                    if (C) 
                    {
                        out = {0, 0, 0};
                        for (unsigned int n = 0; n < demo.size(); ++n)
                        {
                            auto rgb = JzAzBz_to_RGB(demo[n].p); 
                            if (meta[n].w >= 1.0 - std::numeric_limits<Scalar>::min() * 5)
                            {
                                // visualize maximum elevation with inverted colour dots
                                out = (xpix % 3) + (ypix % 3) == 0 ? RGBVec{1,1,1} - rgb : rgb;
                            }
                            else
                            {
                                Scalar brightness = std::pow(std::fmod(meta[n].w * C, 1.0f), 8);
                                out += rgb * brightness;
                            }
                        }
                        contour_map.set_pixel(xpix, ypix,
                                (unsigned char)std::round(std::min(out.x(), 1.0f) * 255),
                                (unsigned char)std::round(std::min(out.y(), 1.0f) * 255),
                                (unsigned char)std::round(std::min(out.z(), 1.0f) * 255));
                    }

                }
            }
            auto stop = std::chrono::high_resolution_clock::now();
            auto usec = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
            std::cout << i << ": Generated " << x * y << " interpolations in " << usec << " microseconds\n" 
                    << "About " << 1000000 * x * y / usec << " interpolations per second" 
                    << std::endl;

    
        auto draw_circles = [&](auto& img)
        {
            image_drawer draw(img);
            draw.pen_width(1);
            
            for (const auto& d : demo)
            {
                const Vec2& v = d.s;
                const RGBVec& c = JzAzBz_to_RGB(d.p);
                draw.pen_color(0,0,0);
                draw.pen_width(1);
                draw.circle(v.x() * x, v.y() * y, 7);
                draw.pen_color(255,255,255);
                draw.circle(v.x() * x, v.y() * y, 5);
                draw.pen_color(c.x() * 255, c.y() * 255, c.z() * 255);
                draw.pen_width(3);
                draw.circle(v.x() * x, v.y() * y, 2);
            }
        };
        draw_circles(colours_map);
        draw_circles(contour_map);
    
        // save the images
        colours_map.save_image(std::string("interpolated_colors") + std::to_string(i) + std::string(".bmp"));
        if (C) contour_map.save_image(std::string("interpolated_colors_weight_contours") + std::to_string(i) + std::string(".bmp"));

        ++i;
    };
    std::apply([&](auto& ... tup) { (( interpolate_and_draw(tup) ), ... ); }, interpolators);

    return 0;
}

FIRE(fired_main)
