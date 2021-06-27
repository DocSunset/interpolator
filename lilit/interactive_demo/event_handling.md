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
    //    auto& para = std::get<2>(tup);
        auto& shader_program = std::get<4>(tup);

        shader_program.state = shader_state;
        shader_program.run();
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
    unsigned int w = 720;
    unsigned int h = 720;
    Demo * grabbed = nullptr;
    Demo * selectd = nullptr;
    Demo * hovered = nullptr;
    const Scalar select_dist = 100.0;
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
        if (min_dist > select_dist / (Scalar)w) return std::make_tuple((Demo *)nullptr, -1);
        else return std::make_tuple(selection, sel_idx);
    }

    template<typename Interpolators>
    void move_grabbed(DemoList& demo, Interpolators& interpolators)
    {
        grabbed->s = mouse;
        auto move = [](auto& demo, auto& tup)
        {
            auto& para = std::get<2>(tup);
            auto& shader = std::get<4>(tup);
            shader.reload(demo, para);
        };
        std::apply([&](auto& ... tuples) {((move(demo, tuples)), ...);}, interpolators);
        redraw = true;
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
        case SDLK_ESCAPE:
            if (grabbed) ungrab();
            if (selectd) unselect();
            break;
        }
        break;
// @/
```

```cpp
// @='handle mouse events'
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
// @/
```
