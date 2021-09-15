#pragma once

namespace Component
{
    struct Flag
    {
        bool state;

        Flag(bool s) : state{s} {}
        operator bool() const {return state;}

        void set() {state = true;}
        void clear() {state = false;}
    };
}
