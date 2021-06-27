```cpp
// @='ui mutators'
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
// @/
```

Currently we ignore all window events except closing the window, which quits
the application, and changes to the screen size, which require the width and
height variables in our shader to be updated. The shader itself calls
`glViewport` and does whatever else is necessary to ensure that the whole
window is drawn to and mouse coordinates are not distorted.

```cpp
// @='handle window events'
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
// @/
```

Keyboard events trigger a variety of effects, most of which are fairly self
explanatory.

```cpp
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
