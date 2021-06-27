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
At the moment, the implementation is being transitioned toward using OpenGL for
most of the interpolation; during this transition, RGB space is used for its
convenience.

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
using CIEXYZVec = Eigen::Vector3f;
using JzAzBzVec = Eigen::Vector3f;

using Demo = Interpolators::Demo<Scalar, ID, Vec2, JzAzBzVec>;
using DemoList = std::vector<Demo>;
template<typename Interpolator>
using Shadr = ShaderInterpolators::AcceleratedInterpolator<Interpolator>;

#define INTERPOLATOR(type, ...) std::make_tuple(type{}, std::vector<type::Meta>{}, std::vector<type::Para>{}, type::Para{__VA_ARGS__}, Shadr<type>{})
auto interpolators = std::make_tuple
        ( INTERPOLATOR(Interpolators::IntersectingNSpheres<Demo>)
        , INTERPOLATOR(Interpolators::InverseDistance<Demo>, 4, 0.001, 0.0, 1.0)
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
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <vector>
#include <tuple>
#include <cstdio>
#include <cstdlib>
#include <SDL.h>
#include <Eigen/Core>
#include <Eigen/LU>
#include "../include/interpolators.h"
#include "../include/shader_interpolators.h"
#include "types.h"
#include "ui.h"

DemoList demo;
UserInterface ui;

void loop()
{
    ui.poll_event_queue(demo, interpolators);

    if (true)//ui.needs_to_redraw())
    {
        unsigned int i = 0;
        auto draw = [](unsigned int& i, auto& tuple)
                {if (i++ == ui.active_interpolator()) ui.draw(tuple, demo);};
        std::apply([&](auto& ... tuples) {((draw(i, tuples)), ...);}, interpolators);
    }
}

int main()
{
    ui.init(demo, interpolators);

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

```cpp
// @#'demo/ui.h'
#ifndef UI_H
#define UI_H

#include <cstddef>
#include <random>
#include <chrono>
#include <SDL.h>
#include <SDL_log.h>
#include <SDL_error.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_events.h>
#include <SDL_opengles2.h>
#include <GLES3/gl3.h>
#include "types.h"
#include "../include/shader_interpolators.h"

@{colour conversions}

class UserInterface
{
public:
    bool ready_to_quit() const {return quit;}
    bool needs_to_redraw() const {return redraw;}
    std::size_t active_interpolator() const {return _active_interpolator;}
    unsigned int width() const {return shader_state.w;}
    unsigned int height() const {return shader_state.h;}

    template<typename Interpolators>
    void init(DemoList& demo, Interpolators& interpolators)
    {
        @{setup}
    }

    template<typename Tuple> void draw(Tuple& tup, const DemoList& demo) const
    {
    //    auto& interpolator = std::get<0>(tup);
    //    auto& meta = std::get<1>(tup);
    //    auto& para = std::get<2>(tup);
        auto& shader_program = std::get<4>(tup);

        shader_program.state = shader_state;
        shader_program.run();

        SDL_GL_SwapWindow(sdl.window);

        redraw = false;
    }

    template<typename Interpolators>
    void poll_event_queue(DemoList& demo, Interpolators& interpolators)
    {
        @{poll event queue and handle events}
    }

private:
    mutable bool redraw = true;

    bool quit = false;
    ShaderInterpolators::ShaderInterpolatorState shader_state = {};
    Vec2 mouse = {0, 0};
    Demo * grabbed = nullptr;
    Demo * selectd = nullptr;
    Demo * hovered = nullptr;
    const Scalar select_dist = 30.0;
    std::size_t _active_interpolator = 0;
    bool fullscreen = false;

    @{SDL declarations}

    @{ui mutators}
};
#endif
// @/
```

Each loop, the SDL event queue is polled until empty, and execution switches
over the type of event. The current implementation is self-explanatory and
provisional, so no further documentation is provided at this time.

```cpp
// @='poll event queue and handle events'
static SDL_Event ev;
while (SDL_PollEvent(&ev)) switch (ev.type)
{
case SDL_QUIT:
case SDL_APP_TERMINATING:
case SDL_APP_LOWMEMORY:
    quit = true;
    break;

@{handle window events}

@{handle keyboard events}

@{handle mouse events}

default:
    break;
}
// @/
```

The details of event handing are found here:

@[lilit/interactive_demo/event_handling.md]

The details of setup are found here:

@[lilit/interactive_demo/setup.md]
