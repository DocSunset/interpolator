```cpp
// @#'demo/ui.h'
#ifndef UI_H
#define UI_H

#include <cstddef>
#include "types.h"

@{colour conversions}

class UserInterface
{
public:
    bool ready_to_quit() const {return quit;}
    bool needs_to_redraw() const {return redraw;}
    std::size_t active_interpolator() const {return _active_interpolator;}
    const Texture& texture() const {return _texture;}

    template<typename Tuple> void draw(const std::size_t i, Tuple& tup, const DemoList& demo) const
    {
        auto& interpolator = std::get<0>(tup);
        auto& meta = std::get<1>(tup);
        auto& para = std::get<2>(tup);
        
        @{run the timer and draw}

        redraw = false;
    }

    void poll_event_queue(DemoList& demo)
    {
        @{poll event queue and handle events}
    }

    mutable Texture _texture = Texture(500, 500);
private:
    mutable bool redraw = true;

    bool quit = false;
    unsigned int contour_lines = 0;
    Vec2 mouse = {0, 0};
    unsigned int w = 500;
    unsigned int h = 500;
    Interpolator::Demo * grabbed = nullptr;
    std::size_t grabbed_idx = 0;
    Scalar grab_dist = 20.0/500.0;
    std::size_t _active_interpolator = 0;
};
#endif
// @/
```

The drawing routine is described in drawing.md. Event handline is described
below.

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

// @='handle window events'
    case SDL_WINDOWEVENT:
        // TODO
        break;
// @/

// @='handle keyboard events'
    case SDL_KEYDOWN:
        contour_lines = 10; 
        redraw = true;
        break;

    case SDL_KEYUP:
        contour_lines = 0; 
        redraw = true;
        break;
// @/

// @='handle mouse events'
    case SDL_MOUSEMOTION:
        mouse = {ev.button.x / (Scalar)w, ev.button.y / (Scalar)h};
        if (grabbed)
        {
            grabbed->s = mouse;
#           ifndef __EMSCRIPTEN__
            redraw = true;
#           endif
        }
        break;

    case SDL_MOUSEBUTTONDOWN:
        mouse = {ev.button.x / (Scalar)w, ev.button.y / (Scalar)h};
        {
            Scalar dist, min_dist;
            min_dist = std::numeric_limits<Scalar>::max();
            for (unsigned int n = 0; n < demo.size(); ++n)
            {
                auto& d = demo[n];
                dist = (mouse - d.s).norm();
                if (dist < min_dist) 
                {
                    grabbed = &d;
                    grabbed_idx = n;
                    min_dist = dist;
                }
            }
            if (min_dist > grab_dist / (Scalar)w) grabbed = nullptr;
        }
        if (contour_lines) redraw = true;
        break;

    case SDL_MOUSEBUTTONUP:
        grabbed = nullptr;
        redraw = true;
        break;

    case SDL_MOUSEWHEEL:
        _active_interpolator = (_active_interpolator + 1) % num_interpolators;
        redraw = true;
        break;
// @/
```