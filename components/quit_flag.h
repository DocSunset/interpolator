#pragma once

namespace Component
{
    struct QuitFlag
    {
        bool state;
        QuitFlag(bool s) : state{s} {}
        operator bool() const {return state;}
    };
}
