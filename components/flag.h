#pragma once

namespace Component
{
    struct Flag
    {
        bool state;

        template<typename ... Ts> Flag(Ts... ts) : state{false} {}

        Flag(bool s) : state{s} {}
        operator bool() const {return state;}

        void set() {state = true;}
        void clear() {state = false;}
        void toggle() {state = !state;}
    };
}
