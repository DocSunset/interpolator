#include <string>
#include <cmath>
#include <chrono>
#include <random>
#include <iostream>
#include <GL/gl.h>
#include <GL/glut.h>
#include <Eigen/Core>
#include <Eigen/LU>
#include "include/fire-hpp/fire.hpp"
#include "../interpolator/marier_spheres.h"

using Scalar = float;
using ID = unsigned int;
using Vec2 = Eigen::Vector2f;
using RGBVec = Eigen::Vector3f;
using CIEXYZVec = Eigen::Vector3f;
using JzAzBzVec = Eigen::Vector3f;
using Interpolator = MarierSpheresInterpolator<Scalar, ID, Vec2, JzAzBzVec>;
using Demo = typename Interpolator::Demo;

Interpolator interpolator;
std::vector<Demo> demos;
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
Vec2 q = {0,0};

void drawCircle(const Vec2& position, const Scalar& radius, const Scalar& depth = 0)
{
    constexpr Scalar pi = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899863;
    for (float w = 0; w <= 2 * pi; w += 0.1)
    {
        glVertex3f(
                position.x() + std::cos(w) * radius, 
                position.y() + std::sin(w) * radius,
                depth);
    }
}

void drawCircleFilled(const Vec2& position, const Scalar& radius, const Scalar& depth = 0)
{
    glBegin(GL_POLYGON);
    drawCircle(position, radius, depth);
    glEnd();
}

void drawCircleWire(const Vec2& position, const Scalar& radius, const Scalar& depth = 0)
{
    glBegin(GL_LINE_LOOP);
    drawCircle(position, radius, depth);
    glEnd();
}

void display()
{
    constexpr float dot_radius = 0.01;
    JzAzBzVec interpolated_jab{0, 0, 0};
    RGBVec q_color = JzAzBz_to_RGB(interpolator.query(q, demos, interpolated_jab));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (const auto& d : demos)
    {
        const auto& v = d.s;
        // draw disk for weight
        glColor4f(d.w, d.w, d.w, d.w);
        drawCircleFilled(v, d.w/3, -1);
    }

    for (const auto& d : demos)
    {
        const auto& v = d.s;
        const auto c = JzAzBz_to_RGB(d.p);
        // draw radial circle
        glColor3f(c.x(), c.y(), c.z());
        drawCircleWire(v, d.r, 1);
        drawCircleWire(v, d.r - 0.001, 1);
    }

    for (const auto& d : demos)
    {
        const auto& v = d.s;
        const auto c = JzAzBz_to_RGB(d.p);
        // draw colored dots
        glColor3f(c.x(), c.y(), c.z());
        drawCircleFilled(v, dot_radius, 1);
    }

    glColor3f(q_color.x(), q_color.y(), q_color.z());
    drawCircleWire(q, interpolator.q_radius);
    drawCircleWire(q, interpolator.q_radius - 0.001);
    drawCircleFilled(q, dot_radius, 1);

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (h < w)
    {   // the window is a wide rectangle
        Scalar padding = (w - h)/2; // there is `padding` space on either size of the scene square
        Scalar pixels_per_unit = h; // this many pixels per unit in world coordinates
        Scalar units_per_pixel = 1/pixels_per_unit;
        Scalar offset = padding * units_per_pixel;
        glOrtho(-offset, 1 + offset, 0,       1, -1, 1);
    }
    else
    {   // the window is a tall rectangle (or square)
        Scalar padding = (h - w)/2; // there is `padding` space above and below the scene square
        Scalar pixels_per_unit = w; // this many pixels per unit in world coordinates
        Scalar units_per_pixel = 1/pixels_per_unit;
        Scalar offset = padding * units_per_pixel;
        glOrtho(0,       1,          -offset, 1 + offset, -1, 1);
    }
}

void mouse_move(int x, int y)
{
    y = glutGet(GLUT_WINDOW_HEIGHT) - y; // flip y so it goes from the bottom left
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    if (h < w)
    {   // the window is a wide rectangle
        Scalar padding = (w - h)/2; // there is `padding` space on either size of the scene square
        Scalar pixels_per_unit = h; // this many pixels per unit in world coordinates
        Scalar units_per_pixel = 1/pixels_per_unit;
        Scalar offset = padding * units_per_pixel;
        q.x() = (Scalar)x / pixels_per_unit - offset;
        q.y() = (Scalar)y / pixels_per_unit;
    }
    else
    {   // the window is a tall rectangle (or square)
        Scalar padding = (h - w)/2; // there is `padding` space above and below the scene square
        Scalar pixels_per_unit = w; // this many pixels per unit in world coordinates
        Scalar units_per_pixel = 1/pixels_per_unit;
        Scalar offset = padding * units_per_pixel;
        q.x() = (Scalar)x / pixels_per_unit;
        q.y() = (Scalar)y / pixels_per_unit - offset;
    }
    glutPostRedisplay();
}

int fired_main(unsigned int n = fire::arg("n", "The number of demonstrations", 5))
{
    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator (seed);
    std::uniform_real_distribution<Scalar> random(0, 1);
    while(n-- > 0)
    {
        auto v = Vec2{random(generator), random(generator)};
        auto c = RGB_to_JzAzBz(RGBVec{random(generator), random(generator), random(generator)});
        Demo d{n, v, c};
        demos.push_back(d);
    }

    // fake argv and argc since we can't access the real ones behind fire-hpp
    char * fake_argv[1];
    std::string prog_name = "interactive_colors";
    fake_argv[0] = &*prog_name.begin();
    int fake_argc = 1;
    
    glutInit(&fake_argc, fake_argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0,0);
    glutInitWindowSize(500,500);
    glutCreateWindow("Interactive Interpolator Demo");
    glutMotionFunc(mouse_move);
    glutPassiveMotionFunc(mouse_move);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();

    glutMainLoop();

    return 0;
}

FIRE(fired_main)