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
        if (grab) move_grabbed();
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
// @/
```

```cpp
// @='selection handlers'
void move_grabbed()
{
    if (selectd.front().type == SelectionType::Demo)
    {
        for (auto& grabbed : selectd) grabbed.demo.d->s += dmouse;
        reload_textures();
        update_slider_bounds();
        redraw = true;
    }
    else if (selectd.front().type == SelectionType::Slider)
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

void select(const Selection& sel)
{
    if (not sel)
    {
        unselect();
        return;
    }

    if (sel.type == SelectionType::Demo)
    {
        if (shift)
        {
            auto found = std::find(selectd.begin(), selectd.end(), sel);
            if (found != selectd.end()) selectd.erase(found);
            else selectd.push_front(sel);
        }
        else
        {
            if (selectd.size() != 0) selectd.clear();
            selectd.push_front(sel);
        }
    }
    else if (sel.type == SelectionType::Slider)
    {
        selectd.clear();
        sel.slider.s->grab = true;
        selectd.push_front(sel);
        set_grabbed_slider();
    }

    if (selectd.front().type == SelectionType::Demo)
    {
        update_slider_values();
        update_slider_bounds();
    }
    unhover();
    grab = true;
}

void hover(const Selection& sel)
{
    if (not sel)
    {
        unhover();
        return;
    }

    unhover();

    if (sel.type == SelectionType::Slider)
        sel.slider.s->hover = true;

    hovered = sel;
}

void ungrab()   
{
    if (not grab) return;
    if (selectd.front().type == SelectionType::Slider)
    {
        set_grabbed_slider();
        grabbed.slider.s->grab = false;
        hover(selectd.front());
    }
    grab = false;
    redraw = true;
}

void unselect()
{
    if (not selectd) return;
    selectd.clear();
}

void unhover()
{
    if (not hovered) return;
    if (hovered.type == SelectionType::Slider) hovered.slider.s->hover = false;
    hovered = Selection::None();
}
// @/
```
