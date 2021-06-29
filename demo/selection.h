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
