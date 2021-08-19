#pragma once

struct QuitFlag
{
    bool state;
    QuitFlag(bool s) : state{s} {}
    operator bool() const {return state;}
};
