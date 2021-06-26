```cpp
// @#'demo/ui.h'
#ifndef UI_H
#define UI_H

#include <cstddef>
#include "types.h"
#include "../include/shader_interpolators.h"

@{colour conversions}

class UserInterface
{
public:
    bool ready_to_quit() const {return quit;}
    bool needs_to_redraw() const {return redraw;}
    std::size_t active_interpolator() const {return _active_interpolator;}
    unsigned int width() const {return w;}
    unsigned int height() const {return h;}

    template<typename Tuple> void draw(Tuple& tup, const DemoList& demo) const
    {
    //    auto& interpolator = std::get<0>(tup);
    //    auto& meta = std::get<1>(tup);
        auto& para = std::get<2>(tup);
        auto& shader_program = std::get<4>(tup);

        if (updated_grab) shader_program.reload(demo, para);
        shader_program.state = shader_state;
        shader_program.run();
        redraw = false;
    }

    void poll_event_queue(DemoList& demo)
    {
        @{poll event queue and handle events}
    }

private:
    mutable bool redraw = true;

    bool quit = false;
    bool updated_grab = false;
    ShaderInterpolators::ShaderInterpolatorState shader_state = {};
    Vec2 mouse = {0, 0};
    unsigned int w = 500;
    unsigned int h = 500;
    Demo * grabbed = nullptr;
    Scalar grab_dist = 100.0;
    std::size_t _active_interpolator = 0;

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

// @='ui mutators'
    void toggle_drawable_flag(bool& flag)
    {
        flag = not flag;
        redraw = true;
    }
    void set_mouse(SDL_Event ev)
    {
        mouse = {ev.motion.x / (Scalar)w, 1.0 - ev.motion.y / (Scalar)h};
    }
    void grab(Demo& d, std::size_t n)
    {
        grabbed = &d;
        shader_state.grabbed_idx = n;
    }
    void ungrab()
    {
        grabbed = nullptr;
        shader_state.grabbed_idx = -1;
    }
// @/

// @='handle window events'
    case SDL_WINDOWEVENT:
        // TODO
        break;
// @/

// @='handle keyboard events'
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
            break;
        case SDLK_f:
            if (ev.type == SDL_KEYUP) break;
            if (ev.key.keysym.mod == KMOD_NONE) toggle_drawable_flag(shader_state.focus);
            break;
        case SDLK_c:
            if (ev.type == SDL_KEYUP) break;
            if (ev.key.keysym.mod == KMOD_NONE) toggle_drawable_flag(shader_state.enable_contours);
            break;
        }
        break;
// @/

// @='handle mouse events'
    case SDL_MOUSEMOTION:
        set_mouse(ev);
        if (grabbed)
        {
            grabbed->s = mouse;
            updated_grab = true;
            redraw = true;
        }
        break;

    case SDL_MOUSEBUTTONDOWN:
        {
            set_mouse(ev);
            Scalar dist, min_dist;
            min_dist = std::numeric_limits<Scalar>::max();
            for (unsigned int n = 0; n < demo.size(); ++n)
            {
                auto& d = demo[n];
                dist = (mouse - d.s).norm();
                if (dist < min_dist) 
                {
                    grab(d, n);
                    min_dist = dist;
                }
            }
            if (min_dist > grab_dist / (Scalar)w) ungrab();
        }
        if (shader_state.enable_contours) redraw = true;
        break;

    case SDL_MOUSEBUTTONUP:
        ungrab();
        redraw = true;
        break;

    case SDL_MOUSEWHEEL:
        break;
// @/
```
