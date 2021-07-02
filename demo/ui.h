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


#define INTERPOLATOR(type, ...) std::make_tuple(type{}, std::vector<type::Meta>{}, std::vector<type::Para>{}, type::Para{__VA_ARGS__}, Shadr<type>{})
auto interpolators = std::make_tuple
        ( INTERPOLATOR(Interpolators::BasicLampshade<Demo>, 2, 100)
        , INTERPOLATOR(Interpolators::IntersectingNSpheres<Demo>)
        , INTERPOLATOR(Interpolators::InverseDistance<Demo>, 4, 0.001, 0.0, 1.0)
        );

class UserInterface
{
public:
    bool ready_to_quit() const {return quit;}

    void init()
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

        unsigned int n = 5;
        unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator (seed);
        std::uniform_real_distribution<Scalar> random(0, 1);
        while(n-- > 0)
        {
            auto v = Vec2{ random(generator) * window.w - window.w/2.0
                         , random(generator) * window.h - window.h/2.0
                         };
            auto c = RGBVec{random(generator), random(generator), random(generator)};
            demo.push_back({v, c, n});
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

        poll_event_queue(); // sets height and width if window was resized immediately, e.g. by a dynamic tiling window manager

        std::size_t max_params = 0;
        auto init_shaders = [&](auto& tup)
        {
            auto& para = std::get<2>(tup);
            auto& shader_program = std::get<4>(tup);
            shader_program.init(demo, para);
            max_params = std::max(para[0].size(), max_params);
        };
        std::apply([&](auto& ... tuples) {((init_shaders(tuples)), ...);}, interpolators);

        slider.resize(max_params + 5);
        for (auto& s : slider) 
        {
            s.init();
            s.window = window;
        }

        slider[0].foreground = {1.0, 0.0, 0.0};
        slider[1].foreground = {0.0, 1.0, 0.0};
        slider[2].foreground = {0.0, 0.0, 1.0};
        change_active_interpolator(0); // sets active_sliders and inits slider bounds and values

        selection_vis.init(demo);
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

        if (true)//redraw);
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
                    selection_vis.window = window;
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
                    if (ev.type == SDL_KEYDOWN) shift = true;
                    else shift = false;
                    break;
                case SDLK_LALT:
                case SDLK_RALT:
                    if (ev.type == SDL_KEYDOWN) alt = true;
                    else alt = false;
                    break;
                case SDLK_LCTRL:
                case SDLK_RCTRL:
                case SDLK_LGUI:
                case SDLK_RGUI:
                    if (ev.type == SDL_KEYDOWN) ctrl = true;
                    else ctrl = false;
                    break;
                case SDLK_LEFT:
                    if (ev.type == SDL_KEYUP) break;
                    if (ev.key.repeat) break;
                    change_active_interpolator(-1);
                    redraw = true;
                    break;
                case SDLK_RIGHT:
                    if (ev.type == SDL_KEYUP) break;
                    if (ev.key.repeat) break;
                    change_active_interpolator(1);
                    redraw = true;
                    break;
                case SDLK_q:
                    if (ev.type == SDL_KEYUP) break;
                    if (ctrl) quit = true;
                    break;
                case SDLK_a:
                    if (ctrl) select_all();
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
                    ungrab();
                    unselect();
                    break;
                }
                break;

            case SDL_MOUSEMOTION:
                set_mouse(ev);
                if (move_grabbed()) break;
                else hover(search_for_selection());
                break;

            case SDL_MOUSEBUTTONDOWN:
                set_mouse(ev);
                select(search_for_selection());
                break;

            case SDL_MOUSEBUTTONUP:
                set_mouse(ev);
                ungrab();
                break;

            case SDL_MOUSEWHEEL:
                break;

        default:
            break;
        }
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
            Vec2 new_mouse = { ev.motion.x - window.w/2.0
                             , window.h/2.0 - ev.motion.y
                             };
            dmouse = new_mouse - mouse;
            mouse = new_mouse;
        }

        void reload_textures()
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

        bool move_grabbed()
        {
            if (grab.type == SelectionType::None) return false;
            else if (grab.type == SelectionType::Demo)
            {
                grab.demo.drop = false;
                for (auto& grabbed : demo_selection) grabbed.demo.d->s += dmouse;
                reload_textures();
                update_slider_bounds();
            }
            else if (grab.type == SelectionType::Slider)
            {
                set_grabbed_slider();
            }
            redraw = true;
            return true;
        }

        Selection search_for_selection()
        {
            Scalar dist, min_dist;
            Selection sel;
            min_dist = std::numeric_limits<Scalar>::max();
            for (std::size_t n = 0; n < demo.size(); ++n)
            {
                auto& d = demo[n];
                dist = (mouse - d.s).norm();
                if (dist < min_dist) 
                {
                    sel.demo.type = SelectionType::Demo;
                    sel.demo.d = &d;
                    sel.demo.idx = n;
                    sel.demo.drop = false;
                    min_dist = dist;
                }
            }
            if (min_dist <= select_dist) 
                return sel;

            for (std::size_t n = 0; n < active_sliders; ++n)
            {
                auto& s = slider[n];
                if (  s.box.left < mouse.x() && s.box.left + s.box.width > mouse.x()
                   && s.box.bottom < mouse.y() && s.box.bottom + s.box.height > mouse.y())
                {
                   sel.slider.type = SelectionType::Slider;
                   sel.slider.s = &s;
                   sel.slider.idx = n;
                   return sel;
                }
            }

            return Selection::None();
        }

        void select_all()
        {
            demo_selection.clear();
            for (std::size_t n = 0; n < demo.size(); ++n)
            {
                Selection sel;
                sel.demo.type = SelectionType::Demo;
                sel.demo.d = &(demo[n]);
                sel.demo.idx = n;
                sel.demo.drop = false;
                demo_selection.push_front(sel);
            }
            update_slider_values();
            update_slider_bounds();
        }

        void select(const Selection& sel)
        {
            if (not sel)
            {
                unselect();
                return;
            }

            grab = sel;
            if (sel.type == SelectionType::Demo)
            {
                auto found = demo_selection.cend();
                for (auto it = demo_selection.cbegin(); it != demo_selection.cend(); ++it)
                {
                    if (it->demo.idx == grab.demo.idx)
                    {
                        found = it;
                        break;
                    }
                }
                if (found == demo_selection.cend()) // if sel not in demo_selection
                {
                    if (not shift) demo_selection.clear();
                    demo_selection.push_front(sel);
                    shader_state.focus_idx = sel.demo.idx;
                }
                else grab.demo.drop = true;
                
                update_slider_values();
                update_slider_bounds();
            }
            else if (sel.type == SelectionType::Slider)
            {
                grab.slider.s->grab = true;
                set_grabbed_slider();
            }
        }

        void unselect()
        {
            demo_selection.clear();
            shader_state.focus_idx = -1;
        }


        void hover(const Selection& sel)
        {
            if (not sel)
            {
                unhover();
                return;
            }

            unhover();

            if (sel.type == SelectionType::Demo && demo_selection.size() == 0)
                shader_state.focus_idx = sel.demo.idx;
            else if (sel.type == SelectionType::Slider)
                sel.slider.s->hover = true;

            hovered = sel;
        }

        void ungrab()   
        {
            if (grab.type == SelectionType::None) return;
            else if (grab.type == SelectionType::Demo && grab.demo.drop)
            {
                if (shift)
                {
                    for (auto it = demo_selection.cbegin(); it != demo_selection.cend(); ++it)
                    {
                        if (it->demo.idx == grab.demo.idx)
                        {
                            demo_selection.erase(it);
                            break;
                        }
                    }
                }
                else
                {
                    demo_selection.clear();
                    demo_selection.push_front(grab);
                }
            }
            else if (grab.type == SelectionType::Slider)
            {
                set_grabbed_slider();
                grab.slider.s->grab = false;
                hover(grab);
            }
            grab = Selection::None();
            redraw = true;
        }

        void unhover()
        {
            if (not hovered) return;
            if (hovered.type == SelectionType::Slider) hovered.slider.s->hover = false;
            else if (hovered.type == SelectionType::Demo && demo_selection.size() == 0) shader_state.focus_idx = -1;
            hovered = Selection::None();
        }

        void update_slider_bounds()
        {
            if (demo_selection.size() == 0) return;
            bool vertical_sliders = window.w >= window.h;
            constexpr float spacing = 10;
            constexpr float slider_width = 30;
            float slider_length = vertical_sliders ?  window.h : 0.5 * window.w;
            slider_length -= 2 * spacing;
            float width  = vertical_sliders ? slider_width  : slider_length;
            float height = vertical_sliders ? slider_length : slider_width; 

            Vec2 centroid = {0,0};
            float count = 0;
            for (auto& sel : demo_selection)
            {
                centroid += sel.demo.d->s;
                count += 1;
            }
            centroid /= count;
            std::size_t start = 0;
            std::size_t incr = 1;
            float step, baseline;
            float slider_step = spacing + slider_width;
            if (vertical_sliders)
            {
                baseline = -window.h/2.0 + spacing;
                if (centroid.x() < 0)
                {
                    start = active_sliders - 1;
                    incr = -1;
                    slider_step = -slider_step;
                    step = window.w/2.0 - spacing - slider_width;
                }
                else step = -window.w/2.0 + spacing;
            }
            else
            {
                if (centroid.y() < 0)
                {
                    start = active_sliders - 1;
                    incr = -1;
                    slider_step = -slider_step;
                    step = window.h/2.0 - spacing - slider_width;
                }
                else step = -window.h/2.0 + spacing;
                baseline = -window.w/2.0 + spacing;
            }

            std::size_t j = start;
            for (std::size_t i = 0; i < active_sliders; ++i)
            {
                if (vertical_sliders) slider[j].box = {baseline, step, height, width};
                else                  slider[j].box = {step, baseline, height, width};
                slider[j].window = window;
                step += slider_step;
                j += incr;
            }

            redraw = true;
        }

        void update_slider_values()
        {
            if (demo_selection.size() == 0) return;

            auto do_update = [&](Demo& d, auto& p)
            {
                if (demo_selection.size() == 1)
                {
                    slider[0].set_value(d.p.x(), 0.0, 1.0);
                    slider[0].link = Slider::Link{0.0, 1.0};
                    slider[1].set_value(d.p.y(), 0.0, 1.0);
                    slider[1].link = Slider::Link{0.0, 1.0};
                    slider[2].set_value(d.p.z(), 0.0, 1.0);
                    slider[2].link = Slider::Link{0.0, 1.0};
                }
                else
                {
                    slider[0].set_value(0.0, 0.0, 1.0);
                    slider[0].link = Slider::Link{0.0, 1.0};
                    slider[1].set_value(0.0, 0.0, 1.0);
                    slider[1].link = Slider::Link{0.0, 1.0};
                    slider[2].set_value(0.0, 0.0, 1.0);
                    slider[2].link = Slider::Link{0.0, 1.0};
                }
                std::size_t slider_idx = 3;
                if constexpr (std::remove_reference_t<decltype(p)>::size() > 0)
                {
                    for (std::size_t i = 0; 
                         i < std::remove_reference_t<decltype(p)>::size(); 
                         ++i)
                    {
                        slider[slider_idx].link = Slider::Link{p.min[i], p.max[i]};
                        slider[slider_idx].set_value(p[i], p.min[i], p.max[i]);
                        slider_idx++;
                    }
                }
            };

            auto update_outer = [&](std::size_t i, auto& tuple)
            {
                if (i != active_interpolator) return;
                auto& para = std::get<2>(tuple);
                do_update(demo[demo_selection.front().demo.idx], para[demo_selection.front().demo.idx]);
            };

            std::size_t i = 0;
            std::apply([&](auto& ... tuples) {((update_outer(i++, tuples)), ...);}, interpolators);
        }

        void change_active_interpolator(int increment)
        {
            if (increment == static_cast<int>(num_interpolators)) return;
            active_interpolator = (active_interpolator + increment) % num_interpolators;
            auto set_active_sliders = [&](std::size_t i, auto& tuple)
            {
                if (i != active_interpolator) return;
                auto& para = std::get<2>(tuple);
                active_sliders = para[0].size() + 3;
            };
            unsigned int i = 0;
            std::apply([&](auto& ... tuples) {((set_active_sliders(i++, tuples)), ...);}, interpolators);
            update_slider_bounds();
            update_slider_values();
        }

        void set_parameter_normalized(Scalar value, std::size_t demo_idx, std::size_t para_idx)
        {
            auto do_update = [&](Demo& d, auto& p)
            {
                if (para_idx < 3) d.p(para_idx) = value;
                else if constexpr (std::remove_reference_t<decltype(p)>::size() > 0)
                {
                    para_idx -= 3;
                    p[para_idx] = value * (p.max[para_idx] - p.min[para_idx]) + p.min[para_idx];
                }
            };

            auto update_outer = [&](std::size_t i, auto& tuple)
            {
                if (i != active_interpolator) return;
                auto& para = std::get<2>(tuple);
                do_update(demo[demo_idx], para[demo_idx]);
            };

            std::size_t i = 0;
            std::apply([&](auto& ... tuples) {((update_outer(i++, tuples)), ...);}, interpolators);
        }

        void set_grabbed_slider()
        {
            if (grab.type != SelectionType::Slider) return;
            if (demo_selection.size() == 0) return;

            Scalar value;
            if (window.w > window.h) // vertical sliders
                value = (mouse.y() - grab.slider.s->box.bottom) / grab.slider.s->box.height;
            else
                value = (mouse.x() - grab.slider.s->box.left) / grab.slider.s->box.width;

            grab.slider.s->normalized_value = value;
            for (auto& s : demo_selection) set_parameter_normalized(value, s.demo.idx, grab.slider.idx);
            reload_textures();
        }
};
#endif
