#include <string>
#include <cmath>
#include <chrono>
#include <random>
#include <iostream>
#include <GL/gl.h>
#include <GL/glut.h>
#include <Eigen/Core>
#include "include/fire-hpp/fire.hpp"
#include "../interpolator/marier_spheres.h"

using Scalar = float;
using ID = unsigned int;
using Vec2 = Eigen::Vector2f;
using RGBVec = Eigen::Vector3f;
using Interpolator = MarierSpheresInterpolator<Scalar, ID, Vec2, RGBVec>;
using Demo = typename Interpolator::Demo;

Interpolator interpolator;
std::vector<Demo> demos;
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
    auto q_color = interpolator.query(q, demos);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (const auto& d : demos)
    {
        const auto& v = d.s;
        // draw disk for weight
        glColor4f(0.2, 0.2, 0.2, 0.1);
        drawCircleFilled(v, d.w/3, -1);
    }

    for (const auto& d : demos)
    {
        const auto& v = d.s;
        const auto& c = d.p;
        // draw radial circle
        glColor3f(c.x(), c.y(), c.z());
        drawCircleWire(v, d.r, 1);
    }

    for (const auto& d : demos)
    {
        const auto& v = d.s;
        const auto& c = d.p;
        // draw colored dots
        glColor3f(c.x(), c.y(), c.z());
        drawCircleFilled(v, dot_radius, 1);
    }

    glColor3f(q_color.x(), q_color.y(), q_color.z());
    drawCircleWire(q, interpolator.q_radius);
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
        auto c = RGBVec{random(generator), random(generator), random(generator)};
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