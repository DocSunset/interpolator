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
