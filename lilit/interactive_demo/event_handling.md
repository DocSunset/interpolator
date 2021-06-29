```cpp
// @='ui mutators'
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

    @{selection handlers}

    void update_slider_bounds()
    {
        if (selectd.type != SelectionType::Demo) return;
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
            if (selectd.demo.d->s.x() < 0)
            {
                slider_step = -slider_step;
                step = window.w/2.0 - spacing - slider_width;
            }
            else step = -window.w/2.0 + spacing;
            baseline = -window.h/2.0 + spacing;
        }
        else
        {
            if (selectd.demo.d->s.y() < 0)
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

        redraw = true;
    }

    template<typename Interpolators>
    void update_slider_values(const DemoList& demo, const Interpolators& interpolators)
    {
        if (selectd.type != SelectionType::Demo) return;

        auto do_update = [&](const Demo& d, const auto& p)
        {
            slider[0].set_value(d->s.x() / window.w + 0.5, 0.0, 1.0);
            slider[1].set_value(d->s.y() / window.h + 0.5, 0.0, 1.0);
            slider[2].set_value(d->p.x(), 0.0, 1.0);
            slider[3].set_value(d->p.y(), 0.0, 1.0);
            slider[4].set_value(d->p.z(), 0.0, 1.0);
            std::size_t slider_idx = 5;
            for (std::size_t i = 0; i < active_sliders - 5; ++i)
                slider[slider_idx++].set_value(p[i], p.min[i], p.max[i]);
        };

        auto update_outer = [&](std::size_t i, const auto& tuple)
        {
            if (i != _active_interpolator) return;
            auto para = std::get<2>(tuple);
            do_update(demo[selectd.demo.idx], para[selected.demo.idx]);
        }

        std::apply([&](auto& ... tuples) {((update_outer(i++, tuples)), ...);}, interpolators);
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
            active_sliders = para[0].size() + 5;
            if (selectd.type == SelectionType::Demo)
                update_slider_values(demo, para);
        };
        unsigned int i = 0;
        std::apply([&](auto& ... tuples) {((set_active_sliders(i++, tuples)), ...);}, interpolators);
        update_slider_bounds();
    }

    void set_grabbed_slider()
    {
        if (grabbed.type != SelectionType::Slider) return;
        if (window.w > window.h) // vertical sliders
        {
            grabbed.slider.s->set_value(
                      mouse.y() - grabbed.slider.s->box.bottom
                    , 0.0f
                    , grabbed.slider.s->box.height
                    );
        }
        else
        {
            grabbed.slider.s->set_value(
                      mouse.x() - grabbed.slider.s->box.left
                    , 0.0f
                    , grabbed.slider.s->box.width
                    );
        }
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
            ungrab();
            unselect();
            break;
        }
        break;
// @/
```

@[lilit/interactive_demo/selection.md]
@[lilit/interactive_demo/slider.md]
