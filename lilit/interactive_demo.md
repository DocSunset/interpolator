# Interactive Interpolator Demonstration

The starring roles are played by the interpolation algorithms, declared in a
tuple alongside the lists of metadata and parameters. The source space is
2D screen coordinates in pixels, shifted so the origin corresponds to the
center of the screen. The destination space is RGB colour.  

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

using Demo = Interpolators::Demo<Scalar, ID, Vec2, RGBVec>;
using DemoList = std::vector<Demo>;
template<typename Interpolator>
using Shadr = ShaderInterpolators::AcceleratedInterpolator<Interpolator>;

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

UserInterface ui;

void loop()
{
    ui.poll_event_queue();

    ui.draw();
}

int main()
{
    ui.init();

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

#include <list>
#include <cstddef>
#include <random>
#include <chrono>
#include <type_traits>
#include <SDL.h>
#include <SDL_log.h>
#include <SDL_error.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_events.h>
#include <SDL_opengles2.h>
#include <GLES3/gl3.h>
#include "types.h"
#include "slider.h"
#include "selection.h"
#include "../include/shader_interpolators.h"

@{colour conversions}

#define INTERPOLATOR(type, ...) std::make_tuple(type{}, std::vector<type::Meta>{}, std::vector<type::Para>{}, type::Para{__VA_ARGS__}, Shadr<type>{})
auto interpolators = std::make_tuple
        ( INTERPOLATOR(Interpolators::SphereLampshade<Demo>, 2, 1, 100)
        , INTERPOLATOR(Interpolators::BasicLampshade<Demo>, 2, 1, 100, 100)
        , INTERPOLATOR(Interpolators::Nodes<Demo>, 500)
        , INTERPOLATOR(Interpolators::IntersectingNSpheres<Demo>)
        , INTERPOLATOR(Interpolators::InverseDistance<Demo>, 2, 0.001, 0.0, 1.0)
        );

class UserInterface
{
public:
    bool ready_to_quit() const {return quit;}

    void init()
    {
        @{setup}
    }

    void draw() const
    {
        auto inner_draw = [&](auto& tup, const DemoList& demo)
        {
            //auto& interpolator = std::get<0>(tup);
            //auto& meta = std::get<1>(tup);
            //auto& para = std::get<2>(tup);
            auto& shader_program = std::get<4>(tup);

            glViewport(0,0,window.w,window.h);

            shader_program.state = shader_state;
            shader_program.window = window;
            shader_program.run();
            selection_vis.run(demo, demo_selection, hovered);
            if (demo_selection.size() > 0) for (std::size_t i = 0; i < active_sliders; ++i) slider[i].run();

            SDL_GL_SwapWindow(sdl.window);

            redraw = false;
        };

        if (redraw)
        {
            unsigned int i = 0;
            auto outer_draw = [&](unsigned int& i, auto& tuple)
            {
                if (i++ == active_interpolator) inner_draw(tuple, demo);
            };
            std::apply([&](auto& ... tuples) {((outer_draw(i, tuples)), ...);}, interpolators);
        }
    }

    void poll_event_queue()
    {
        @{poll event queue and handle events}
    }

private:
    const std::size_t num_interpolators = std::tuple_size_v<decltype(interpolators)>;

    DemoList demo;

    mutable bool redraw = true;

    bool quit = false;
    ShaderInterpolators::ShaderInterpolatorState shader_state = {};
    WindowSize window;
    Vec2 mouse = {0, 0};
    Vec2 dmouse = {0, 0};
    Selection grab = Selection::None();
    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    std::list<Selection> demo_selection;
    Selection hovered = Selection::None();
    const Scalar select_dist = 30.0;
    std::size_t active_interpolator = 0;
    bool fullscreen = false;
    std::vector<Slider> slider;
    std::size_t active_sliders = 0;
    SelectionVisualizer selection_vis;

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
