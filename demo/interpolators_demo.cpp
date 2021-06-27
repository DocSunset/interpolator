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
