```cpp
// @='ui mutators'
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

    @{selection handlers}

    void update_slider_bounds()
    {
        if (selectd.front().type != SelectionType::Demo) return;
        bool vertical_sliders = window.w >= window.h;
        constexpr float spacing = 10;
        constexpr float slider_width = 30;
        float slider_length = vertical_sliders ?  window.h : 0.5 * window.w;
        slider_length -= 2 * spacing;
        float width  = vertical_sliders ? slider_width  : slider_length;
        float height = vertical_sliders ? slider_length : slider_width; 

        std::size_t start = 0;
        std::size_t incr = 1;
        float step, baseline;
        float slider_step = spacing + slider_width;
        if (vertical_sliders)
        {
            baseline = -window.h/2.0 + spacing;
            if (selectd.front().demo.d->s.x() < 0)
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
            if (selectd.front().demo.d->s.y() < 0)
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
        if (selectd.front().type != SelectionType::Demo) return;

        auto do_update = [&](Demo& d, auto& p)
        {
            if (selectd.size() == 1)
            {
                slider[0].set_value(d.p.x(), 0.0, 1.0);
                slider[0].link = Slider::Link{0.0, 1.0, d.p.data()};
                slider[1].set_value(d.p.y(), 0.0, 1.0);
                slider[1].link = Slider::Link{0.0, 1.0, d.p.data() + 1};
                slider[2].set_value(d.p.z(), 0.0, 1.0);
                slider[2].link = Slider::Link{0.0, 1.0, d.p.data() + 2};
            }
            else
            {
                slider[0].set_value(0.0, 0.0, 1.0);
                slider[0].link = Slider::Link{0.0, 1.0, nullptr};
                slider[1].set_value(0.0, 0.0, 1.0);
                slider[1].link = Slider::Link{0.0, 1.0, nullptr};
                slider[2].set_value(0.0, 0.0, 1.0);
                slider[2].link = Slider::Link{0.0, 1.0, nullptr};
            }
            std::size_t slider_idx = 3;
            if constexpr (std::remove_reference_t<decltype(p)>::size() > 0)
            {
                for (std::size_t i = 0; 
                     i < std::remove_reference_t<decltype(p)>::size(); 
                     ++i)
                {
                    slider[slider_idx].link = Slider::Link{p.min[i], p.max[i], p.data + i};
                    slider[slider_idx].set_value(p[i], p.min[i], p.max[i]);
                    slider_idx++;
                }
            }
        };

        auto update_outer = [&](std::size_t i, auto& tuple)
        {
            if (i != active_interpolator) return;
            auto& para = std::get<2>(tuple);
            do_update(demo[selectd.front().demo.idx], para[selectd.front().demo.idx]);
        };

        std::size_t i = 0;
        std::apply([&](auto& ... tuples) {((update_outer(i++, tuples)), ...);}, interpolators);
    }

    void change_active_interpolator(int increment)
    {
        if (increment == num_interpolators) return;
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

    void set_grabbed_slider()
    {
        if (selectd.front().type != SelectionType::Slider) return;
        for (auto& grabbed : selectd)
        {
            if (window.w > window.h) // vertical sliders
            {
                grabbed.slider.s->set_value(mouse.y() - grabbed.slider.s->box.bottom
                        , 0.0f
                        , grabbed.slider.s->box.height
                        , grabbed.slider.s->link.dest
                        );
            }
            else
            {
                grabbed.slider.s->set_value(
                          mouse.x() - grabbed.slider.s->box.left
                        , 0.0f
                        , grabbed.slider.s->box.width
                        , grabbed.slider.s->link.dest
                        );
            }
        }
        reload_textures();
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
