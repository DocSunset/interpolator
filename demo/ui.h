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
                , width() , height()
                , SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
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
            auto v = Vec2{ random(generator) * width() - width()/2.0
                         , random(generator) * height() - height()/2.0
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

        auto init_shaders = [&](auto& tup)
        {
            auto& para = std::get<2>(tup);
            auto& shader_program = std::get<4>(tup);
            shader_program.init(demo, para);
        };
        std::apply([&](auto& ... tuples) {((init_shaders(tuples)), ...);}, interpolators);
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
                    shader_state.w = ev.window.data1;
                    shader_state.h = ev.window.data2;
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
                    _active_interpolator = (_active_interpolator - 1) % num_interpolators;
                    redraw = true;
                    break;
                case SDLK_RIGHT:
                    if (ev.type == SDL_KEYUP) break;
                    if (ev.key.repeat) break;
                    _active_interpolator = (_active_interpolator + 1) % num_interpolators;
                    redraw = true;
                    break;
                case SDLK_q:
                    if (ev.type == SDL_KEYUP) break;
                    if (ev.key.keysym.mod & KMOD_CTRL) quit = true;
                    if (ev.key.keysym.mod & KMOD_GUI)  quit = true;
                    break;
                case SDLK_f:
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
                if (grabbed)
                {
                    move_grabbed(demo, interpolators);
                }
                else
                {
                    auto [demoptr, demoidx] = search_for_selection(demo);
                    if (demoptr) hover(demoptr, demoidx);
                    else unhover();
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                {
                    set_mouse(ev);
                    auto [demoptr, demoidx] = search_for_selection(demo);
                    if (demoptr)
                    {
                        grab(demoptr, demoidx);
                    }
                    else unselect();
                }
                break;

            case SDL_MOUSEBUTTONUP:
                ungrab();
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
    Vec2 mouse = {0, 0};
    Demo * grabbed = nullptr;
    Demo * selectd = nullptr;
    Demo * hovered = nullptr;
    const Scalar select_dist = 30.0;
    std::size_t _active_interpolator = 0;

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
            mouse = { ev.motion.x - shader_state.w/2.0
                    , shader_state.h/2.0 - ev.motion.y
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
};
#endif
