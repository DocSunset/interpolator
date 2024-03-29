#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    struct Knob : public System
    {
        void setup_reactive_systems(entt::registry&) override;
        void prepare_to_paint(entt::registry&) override;
        ~Knob();
    private:
        entt::observer updated_knobs;
        entt::observer dragged;
    };
}
