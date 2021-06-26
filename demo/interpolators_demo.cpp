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
#include "../include/shader_interpolators.h"
#include "types.h"
#include "ui.h"

DemoList demo;
UserInterface ui;

struct
{
    SDL_Window * window = nullptr;
    SDL_GLContext gl = nullptr;
} sdl;

void loop()
{
    ui.poll_event_queue(demo);

    if (ui.needs_to_redraw())
    {
        unsigned int i = 0;
        auto draw = [](unsigned int& i, auto& tuple) 
                {if (i++ == ui.active_interpolator()) ui.draw(tuple, demo);};
        std::apply([&](auto& ... tuples) {((draw(i, tuples)), ...);}, interpolators);

        SDL_GL_SwapWindow(sdl.window);
    }
}

int main()
{
    ui = UserInterface{};
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "Error initializing SDL:\n    %s\n", 
                SDL_GetError());
        return EXIT_FAILURE;
    }
    else SDL_Log("Initialized successfully\n");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    sdl.window = SDL_CreateWindow
            ( "Interpolators"
            , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED
            , 500 , 500
            , SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN
            );
    if (sdl.window == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "Error creating window:\n    %s\n", 
                SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                "Couldn't create the main window :(", NULL);
        return EXIT_FAILURE;
    }
    else SDL_Log("Created window\n");

    sdl.gl = SDL_GL_CreateContext(sdl.window);
    if (sdl.gl == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "Error creating OpenGL context:\n    %s\n", 
                SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                "Couldn't create OpenGL context :(", NULL);
        return EXIT_FAILURE;
    }
    else SDL_Log("Created GL context\n");

    unsigned int n = 3;
    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator (seed);
    std::uniform_real_distribution<Scalar> random(0, 1);
    while(n-- > 0)
    {
        auto v = Vec2{random(generator), random(generator)};
        auto c = RGBVec{random(generator), random(generator), random(generator)};
        demo.push_back({n, v, c});
    }

    auto resize_lists = [&](auto& tup)
    {
        auto& meta = std::get<1>(tup);
        auto& para = std::get<2>(tup);
        auto& default_para = std::get<3>(tup);
        meta.resize(demo.size());
        for (auto& m : meta) m = {};
        para.resize(demo.size());
        for (auto& p : para) p = default_para;
    };
    std::apply([&](auto& ... tuples) {((resize_lists(tuples)), ...);}, interpolators);

    auto init_shaders = [&](auto& tup)
    {
        auto& para = std::get<2>(tup);
        auto& shader_program = std::get<4>(tup);
        shader_program.init(demo, para);
    };
    std::apply([&](auto& ... tuples) {((init_shaders(tuples)), ...);}, interpolators);

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
