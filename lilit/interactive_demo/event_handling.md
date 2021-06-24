# Event Handling

```cpp
// @='poll event queue and handle events'
static SDL_Event ev;
while (SDL_PollEvent(&ev)) switch (ev.type)
{
case SDL_QUIT:
case SDL_APP_TERMINATING:
case SDL_APP_LOWMEMORY:
    context.quit = true;
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
        context.C = 10; 
        context.redraw = true;
        break;

    case SDL_KEYUP:
        context.C = 0; 
        context.redraw = true;
        break;
// @/

// @='handle mouse events'
    case SDL_MOUSEMOTION:
        context.mouse = {ev.button.x / (Scalar)context.w, ev.button.y / (Scalar)context.h};
        if (context.grabbed)
        {
            context.grabbed->s = context.mouse;
#           ifndef __EMSCRIPTEN__
            context.redraw = true;
#           endif
        }
        break;

    case SDL_MOUSEBUTTONDOWN:
        context.mouse = {ev.button.x / (Scalar)context.w, ev.button.y / (Scalar)context.h};
        {
            Scalar dist, min_dist;
            min_dist = std::numeric_limits<Scalar>::max();
            for (unsigned int n = 0; n < context.N; ++n)
            {
                auto& d = context.demo[n];
                dist = (context.mouse - d.s).norm();
                if (dist < min_dist) 
                {
                    context.grabbed = &d;
                    context.grabbed_idx = n;
                    min_dist = dist;
                }
            }
            if (min_dist > context.grab_dist / (Scalar)context.w) context.grabbed = nullptr;
        }
        if (context.C) context.redraw = true;
        break;

    case SDL_MOUSEBUTTONUP:
        context.grabbed = nullptr;
        context.redraw = true;
        break;

    case SDL_MOUSEWHEEL:
        context.active_interpolator = (context.active_interpolator + 1) % context.num_interpolators;
        context.redraw = true;
        break;
// @/
```

