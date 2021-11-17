#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    struct Knob : public System
    {
        void setup_reactive_systems(entt::registry&) override;
        void run(entt::registry&) override;
        ~Knob();
    private:
        entt::observer dragged;
    };
}
