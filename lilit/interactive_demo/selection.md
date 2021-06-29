```cpp
// @#'demo/selection.h'
#ifndef SELECTION_H
#define SELECTION_H

#include "types.h"
#include "slider.h"

enum class SelectionType
    { Demo
    , Slider
    , None
    };

struct DemoSelection
{
    SelectionType type;
    Demo * d;
    int idx;
};

struct SliderSelection
{
    SelectionType type;
    Slider * s;
};

struct NoSelection
{
    SelectionType type;
};

union Selection
{
    SelectionType type;
    DemoSelection demo;
    SliderSelection slider;

    static Selection None()
    {
        Selection sel;
        sel.type = SelectionType::None;
        return sel;
    }

    operator bool() const {return not (type == SelectionType::None);}
};
#endif
// @/
```

```cpp
// @='handle mouse events'
    case SDL_MOUSEMOTION:
        set_mouse(ev);
        if (grabbed) move_grabbed();
        else hover(search_for_selection());
        break;

    case SDL_MOUSEBUTTONDOWN:
        set_mouse(ev);
        {
            auto sel = search_for_selection();
            if (sel) grab(sel);
            else unselect();
        }
        break;

    case SDL_MOUSEBUTTONUP:
        set_mouse(ev);
        ungrab();
        break;

    case SDL_MOUSEWHEEL:
        break;
// @/
```

```cpp
// @='selection handlers'
void move_grabbed()
{
    if (grabbed.type == SelectionType::Demo)
    {
        grabbed.demo.d->s = mouse;
        reload_textures();
        update_slider_bounds();
        redraw = true;
    }
    else if (grabbed.type == SelectionType::Slider)
    {
        set_grabbed_slider();
        redraw = true;
    }
}

Selection search_for_selection()
{
    Scalar dist, min_dist;
    Selection sel;
    min_dist = std::numeric_limits<Scalar>::max();
    for (unsigned int n = 0; n < demo.size(); ++n)
    {
        auto& d = demo[n];
        dist = (mouse - d.s).norm();
        if (dist < min_dist) 
        {
            sel.demo.type = SelectionType::Demo;
            sel.demo.d = &d;
            sel.demo.idx = n;
            min_dist = dist;
        }
    }
    if (min_dist <= select_dist) 
        return sel;

    for (auto& s : slider)
    {
        if (  s.box.left < mouse.x() && s.box.left + s.box.width > mouse.x()
           && s.box.bottom < mouse.y() && s.box.bottom + s.box.height > mouse.y())
        {
           sel.slider.type = SelectionType::Slider;
           sel.slider.s = &s;
           return sel;
        }
    }

    return Selection::None();
}

void set_slot(const Selection& sel, Selection& slot, int& idx_slot)
{
    slot = sel;
    if (sel.type == SelectionType::Demo)
    {
        idx_slot = sel.demo.idx;
        redraw = true;
    }
}

void unset_slot(Selection& sel, int& idx_slot)
{
    if (idx_slot >= 0)
    {
        idx_slot = -1;
        redraw = true;
    }
    sel = Selection::None();
}

void grab(const Selection& sel)
{
    if (not sel)
    {
        ungrab();
        return;
    }
    set_slot(sel, grabbed, shader_state.grabbed_idx);
    if (sel.type == SelectionType::Demo)
    {
        select(sel);
        update_slider_bounds();
    }
    if (sel.type == SelectionType::Slider)
    {
        sel.slider.s->grab = true;
        set_grabbed_slider();
    }
    unhover();
}

void select(const Selection& sel)
{
    if (not sel)
    {
        unselect();
        return;
    }

    if (selectd) unselect();

    set_slot(sel, selectd, shader_state.selectd_idx);

    if (selectd.type == SelectionType::Demo)
        update_slider_values();
}

void hover(const Selection& sel)
{
    if (not sel)
    {
        unhover();
        return;
    }

    if (hovered) unhover();

    if (sel.type == SelectionType::Slider)
        sel.slider.s->hover = true;

    set_slot(sel, hovered, shader_state.hovered_idx);
}

void ungrab()   
{
    if (not grabbed) return;
    if (grabbed.type == SelectionType::Slider)
    {
        set_grabbed_slider();
        grabbed.slider.s->grab = false;
        hover(grabbed);
    }
    unset_slot(grabbed, shader_state.grabbed_idx);
    redraw = true;
}

void unselect()
{
    if (not selectd) return;
    unset_slot(selectd, shader_state.selectd_idx);
}

void unhover()
{
    if (not hovered) return;
    if (hovered.type == SelectionType::Slider) hovered.slider.s->hover = false;
    unset_slot(hovered, shader_state.hovered_idx);
}
// @/
```
