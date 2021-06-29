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
#include "slider.h"
#include "../include/shader_interpolators.h"


class UserInterface
{
public:
    bool ready_to_quit() const {return quit;}
    bool needs_to_redraw() const {return redraw;}
    std::size_t active_interpolator() const {return _active_interpolator;}

    template<typename Interpolators>
    void init(DemoList& demo, Interpolators& interpolators)
    {
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                    "Error initializing SDL:\n    %s\n", 
                    SDL_GetError());
            exit(EXIT_FAILURE);
        }
        else SDL_Log("Initialized successfully\n");

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        sdl.window = SDL_CreateWindow
                ( "Interpolators"
                , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED
                , window.w , window.h
                , SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
                );
        if (sdl.window == nullptr)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                    "Error creating window:\n    %s\n", 
                    SDL_GetError());
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                    "Couldn't create the main window :(", NULL);
            exit(EXIT_FAILURE);
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
            exit(EXIT_FAILURE);
        }
        else SDL_Log("Created GL context\n");

        unsigned int n = 3;
        unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator (seed);
        std::uniform_real_distribution<Scalar> random(0, 1);
        while(n-- > 0)
        {
            auto v = Vec2{ random(generator) * window.w - window.w/2.0
                         , random(generator) * window.h - window.h/2.0
                         };
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

        poll_event_queue(demo, interpolators); // sets height and width if window was resized immediately, e.g. by a dynamic tiling window manager

        std::size_t max_params = 0;
        auto init_shaders = [&](auto& tup)
        {
            auto& para = std::get<2>(tup);
            auto& shader_program = std::get<4>(tup);
            shader_program.init(demo, para);
            max_params = std::max(para[0].size(), max_params);
        };
        std::apply([&](auto& ... tuples) {((init_shaders(tuples)), ...);}, interpolators);

        slider.resize(max_params);
        for (auto& s : slider) s.init();
    }

    template<typename Tuple> void draw(Tuple& tup, const DemoList& demo) const
    {
    //    auto& interpolator = std::get<0>(tup);
    //    auto& meta = std::get<1>(tup);
    //    auto& para = std::get<2>(tup);
        auto& shader_program = std::get<4>(tup);

        glViewport(0,0,window.w,window.h);

        shader_program.state = shader_state;
        shader_program.window = window;
        shader_program.run();
        if (selectd) for (std::size_t i = 0; i < active_sliders; ++i) slider[i].run();

        SDL_GL_SwapWindow(sdl.window);

        redraw = false;
    }

    template<typename Interpolators>
    void poll_event_queue(DemoList& demo, Interpolators& interpolators)
    {
        static SDL_Event ev;
        while (SDL_PollEvent(&ev)) switch (ev.type)
        {
        case SDL_QUIT:
        case SDL_APP_TERMINATING:
        case SDL_APP_LOWMEMORY:
            quit = true;
            break;

            case SDL_WINDOWEVENT:
                switch (ev.window.event)
                {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    window.w = ev.window.data1;
                    window.h = ev.window.data2;
                    for (auto& s : slider) s.window = window;
                    update_slider_bounds();
                    redraw = true;
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    quit = true;
                    break;
                }
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                switch (ev.key.keysym.sym)
                {
                case SDLK_LSHIFT:
                case SDLK_RSHIFT:
                    toggle_drawable_flag(shader_state.enable_contours);
                    break;
                case SDLK_LEFT:
                    if (ev.type == SDL_KEYUP) break;
                    if (ev.key.repeat) break;
                    change_active_interpolator(-1, interpolators);
                    redraw = true;
                    break;
                case SDLK_RIGHT:
                    if (ev.type == SDL_KEYUP) break;
                    if (ev.key.repeat) break;
                    change_active_interpolator(1, interpolators);
                    redraw = true;
                    break;
                case SDLK_q:
                    if (ev.type == SDL_KEYUP) break;
                    if (ev.key.keysym.mod & KMOD_CTRL) quit = true;
                    if (ev.key.keysym.mod & KMOD_GUI)  quit = true;
                    break;
                #ifndef __EMSCRIPTEN__
                case SDLK_f:
                    if (ev.type == SDL_KEYUP) break;
                    if (ev.key.keysym.mod == KMOD_NONE)
                    {
                        if (fullscreen)
                        {
                            SDL_SetWindowFullscreen(sdl.window, 0);
                            fullscreen = false;
                        }
                        else
                        {
                            SDL_SetWindowFullscreen(sdl.window, SDL_WINDOW_FULLSCREEN);
                            fullscreen = true;
                        }
                    }
                    break;
                #endif
                case SDLK_x:
                    if (ev.type == SDL_KEYUP) break;
                    if (ev.key.keysym.mod == KMOD_NONE) toggle_drawable_flag(shader_state.focus);
                    break;
                case SDLK_c:
                    if (ev.type == SDL_KEYUP) break;
                    if (ev.key.keysym.mod == KMOD_NONE) toggle_drawable_flag(shader_state.enable_contours);
                    break;
                case SDLK_ESCAPE:
                    if (grabbed) ungrab();
                    if (selectd) unselect();
                    break;
                }
                break;

            case SDL_MOUSEMOTION:
                set_mouse(ev);
                if (active_sliders > 0) update_slider_bounds();
                if (grabbed)
                {
                    move_grabbed(demo, interpolators);
                    update_slider_bounds();
                }
                else
                {
                    auto [demoptr, demoidx] = search_for_selection(demo);
                    if (demoptr) hover(demoptr, demoidx);
                    else 
                    {
                        unhover();
                        if (grabbed_slider) 
                        {
                            set_grabbed_slider();
                        }
                        else
                        {
                            Slider * s = search_for_slider();
                            if (s) s->hover = true;
                            else for (auto& s : slider) s.hover = false;
                        }
                    }
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                {
                    set_mouse(ev);
                    auto [demoptr, demoidx] = search_for_selection(demo);
                    if (demoptr)
                    {
                        grab(demoptr, demoidx);
                        update_slider_bounds();
                    }
                    else
                    {
                        grabbed_slider = search_for_slider();
                        if (grabbed_slider)
                        {
                            grabbed_slider->grab = true;
                            set_grabbed_slider();
                        }
                        else unselect();
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                set_mouse(ev);
                ungrab();
                if (grabbed_slider)
                {
                    set_grabbed_slider();
                    grabbed_slider->grab = false;
                    grabbed_slider = nullptr;
                }
                redraw = true;
                break;

            case SDL_MOUSEWHEEL:
                break;

        default:
            break;
        }
    }

private:
    mutable bool redraw = true;

    bool quit = false;
    ShaderInterpolators::ShaderInterpolatorState shader_state = {};
    WindowSize window;
    Vec2 mouse = {0, 0};
    Slider * grabbed_slider = nullptr;
    Demo * grabbed = nullptr;
    Demo * selectd = nullptr;
    Demo * hovered = nullptr;
    const Scalar select_dist = 30.0;
    std::size_t _active_interpolator = 0;
    bool fullscreen = false;
    std::vector<Slider> slider;
    std::size_t active_sliders = 0;

    struct
    {
        SDL_Window * window = nullptr;
        SDL_GLContext gl = nullptr;
    } sdl;

        void toggle_drawable_flag(bool& flag)
        {
            flag = not flag;
            redraw = true;
        }

        void set_mouse(SDL_Event ev)
        {
            mouse = { ev.motion.x - window.w/2.0
                    , window.h/2.0 - ev.motion.y
                    };
        }

        std::tuple<Demo*, int> search_for_selection(DemoList& demo) const
        {
            Scalar dist, min_dist;
            Demo * selection = nullptr;
            int sel_idx = -1;
            min_dist = std::numeric_limits<Scalar>::max();
            for (unsigned int n = 0; n < demo.size(); ++n)
            {
                auto& d = demo[n];
                dist = (mouse - d.s).norm();
                if (dist < min_dist) 
                {
                    selection = &d;
                    sel_idx = n;
                    min_dist = dist;
                }
            }
            if (min_dist > select_dist) return std::make_tuple((Demo *)nullptr, -1);
            else return std::make_tuple(selection, sel_idx);
        }

        Slider * search_for_slider()
        {
            for (auto& s : slider)
            {
                if (  s.box.left < mouse.x() && s.box.left + s.box.width > mouse.x()
                   && s.box.bottom < mouse.y() && s.box.bottom + s.box.height > mouse.y())
                   return &s;
            }
            return nullptr;
        }

        template<typename Interpolators>
        void reload_textures(DemoList& demo, Interpolators& interpolators)
        {
            auto move = [](auto& demo, auto& tup)
            {
                auto& para = std::get<2>(tup);
                auto& shader = std::get<4>(tup);
                shader.reload(demo, para);
            };
            std::apply([&](auto& ... tuples) {((move(demo, tuples)), ...);}, interpolators);
            redraw = true;
        }

        template<typename Interpolators>
        void move_grabbed(DemoList& demo, Interpolators& interpolators)
        {
            grabbed->s = mouse;
            reload_textures(demo, interpolators);
        }

        void set_slot(Demo* d, int idx, Demo*& slot, int& idx_slot)
        {
            slot = d;
            idx_slot = idx;
        }

        void unset_slot(Demo*& slot, int& idx_slot)
        {
            slot = nullptr;
            idx_slot = -1;
        }

        void grab(Demo* d, int idx)
        {
            set_slot(d, idx, grabbed, shader_state.grabbed_idx);
            select(d, idx);
            unhover();
        }

        void select(Demo* d, int idx)
        {
            set_slot(d, idx, selectd, shader_state.selectd_idx);
            if (shader_state.focus) redraw = true;
        }

        void hover(Demo* d, int idx)
        {
            set_slot(d, idx, hovered, shader_state.hovered_idx);
            if (shader_state.focus) redraw = true;
        }

        void ungrab()   
        {
            unset_slot(grabbed, shader_state.grabbed_idx);
        }

        void unselect()
        {
            unset_slot(selectd, shader_state.selectd_idx);
            if (shader_state.focus) redraw = true;
        }

        void unhover()
        {
            unset_slot(hovered, shader_state.hovered_idx);
        }

        void update_slider_bounds()
        {
            if (selectd == nullptr) return;
            bool vertical_sliders = window.w >= window.h;
            constexpr float spacing = 10;
            constexpr float slider_width = 30;
            float slider_length = vertical_sliders ?  window.h : 0.5 * window.w;
            slider_length -= 2 * spacing;
            float width  = vertical_sliders ? slider_width  : slider_length;
            float height = vertical_sliders ? slider_length : slider_width; 

            float step, baseline;
            float slider_step = spacing + slider_width;
            if (vertical_sliders)
            {
                if (selectd->s.x() < 0)
                {
                    slider_step = -slider_step;
                    step = window.w/2.0 - spacing - slider_width;
                }
                else step = -window.w/2.0 + spacing;
                baseline = -window.h/2.0 + spacing;
            }
            else
            {
                if (selectd->s.y() < 0)
                {
                    slider_step = -slider_step;
                    step = window.h/2.0 - spacing - slider_width;
                }
                else step = -window.h/2.0 + spacing;
                baseline = -window.w/2.0 + spacing;
            }

            for (std::size_t i = 0; i < active_sliders; ++i)
            {
                if (vertical_sliders) slider[i].box = {baseline, step, height, width};
                else                  slider[i].box = {step, baseline, height, width};
                slider[i].window = window;
                step += slider_step;
            }
        }

        template<typename Interpolators>
        void change_active_interpolator(int increment, Interpolators& interpolators)
        {
            if (increment == num_interpolators) return;
            _active_interpolator = (_active_interpolator + increment) % num_interpolators;
            auto set_active_sliders = [&](std::size_t i, auto& tuple)
            {
                if (i != _active_interpolator) return;
                auto& para = std::get<2>(tuple);
                active_sliders = para[0].size();
            };
            unsigned int i = 0;
            std::apply([&](auto& ... tuples) {((set_active_sliders(i++, tuples)), ...);}, interpolators);
            update_slider_bounds();
        }

        void set_grabbed_slider()
        {
            if (window.w > window.h) // vertical sliders
            {
                grabbed_slider->set_value(
                          mouse.y() - grabbed_slider->box.bottom
                        , 0.0f
                        , grabbed_slider->box.height
                        );
            }
            else
            {
                grabbed_slider->set_value(
                          mouse.x() - grabbed_slider->box.left
                        , 0.0f
                        , grabbed_slider->box.width
                        );
            }
        }
};
#endif
