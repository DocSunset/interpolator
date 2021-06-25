# Interactive Interpolator Demonstration

The starring roles are played by the interpolation algorithms, declared in a
tuple alongside the lists of metadata and parameters. The source space is
normalized 2D screen coordinates. The destination space is color.  The colour
interpolations are performed in [J_zA_zB_z colour
space](https://doi.org/10.1364/OE.25.015131), a colour representation that is
designed to provide the best balance of perceptual uniformity and iso-hue
linearity.  This is meant to ensure that the image produced reflects the
topology of the interpolation rather than the non-linearity of sRGB colour
space. The details of the conversion from RGB (assumed sRGB) through CIE XYZ to
J_zA_zB_z colour space and back are given in the file @[lilit/colour_space.md].

```cpp 
// @#'demo/types.h'
#ifndef TYPES_H
#define TYPES_H

#include <vector>
@{include eigen}
#include "../include/interpolators.h"

using Scalar = float;
using ID = unsigned int;
using Vec2 = Eigen::Vector2f;
using RGBVec = Eigen::Vector3f;
using RGBAVec = Eigen::Vector4f;
using CIEXYZVec = Eigen::Vector3f;
using JzAzBzVec = Eigen::Vector3f;
using Texture = Eigen::Matrix<RGBAVec, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

using Interpolator = Interpolators<Scalar, ID, Vec2, JzAzBzVec>;
using DemoList = std::vector<Interpolator::Demo>;

#define INTERPOLATOR(type, ...) std::make_tuple(type{}, std::vector<type::Meta>{}, std::vector<type::Para>{}, type::Para{__VA_ARGS__})
auto interpolators = std::make_tuple
        ( INTERPOLATOR(Interpolator::IntersectingNSpheres)
        , INTERPOLATOR(Interpolator::InverseDistance, 4, 0.001, 0.0)
        );

const std::size_t num_interpolators = std::tuple_size_v<decltype(interpolators)>;

#endif
// @/
```

An overview of the program shows a typical event-loop driven SDL application.
In order to accomodate both Emscripten/web and native SDL build targets, the
loop is placed in a seperate function `void loop()` that can be called by the
browser's event loop or the main function loop depending on the platform.

```cpp
// @#'demo/interpolators_demo.cpp'
@{includes}
@{declare interpolators}

DemoList demo;
UserInterface ui;

@{SDL declarations}
@{openGL declarations}

void loop()
{
    ui.poll_event_queue(demo);

    if (ui.needs_to_redraw())
    {
        @{draw the active interpolator}

        write_gl_texture(ui.texture(), gl.texture);
    }

    glUseProgram(gl.prog);
    glBindVertexArray(Fullscreen::vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, Fullscreen::quad.size());
    SDL_GL_SwapWindow(sdl.window);
}

int main()
{
    ui = UserInterface{};
    @{setup}

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, -1, 1);
#else
    while (not ui.ready_to_quit())
    {
        loop();
        SDL_Delay(33);
    }
#endif

    return EXIT_SUCCESS;
}
// @/
```

There are several main conceptual section or thematic areas in the program, and
the details of implementation of each is broken out into a seperate file: 

@[lilit/interactive_demo/drawing.md]
@[lilit/interactive_demo/event_handling.md]
@[lilit/interactive_demo/setup.md]
@[lilit/interactive_demo/opengl.md]

# ETC

These bits should arguably be shuffled away somewhere else.

```cpp
// @+'includes'
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <vector>
#include <tuple>
#include <random>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <SDL.h>
#include <SDL_log.h>
#include <SDL_error.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_events.h>
#include <SDL_opengles2.h>
#include <GLES3/gl3.h>
#include <Eigen/Core>
#include <Eigen/LU>
#include "../include/interpolators.h"
#include "types.h"
#include "ui.h"
#include "graphics.h"
// @/
```
