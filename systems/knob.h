#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    struct Knob : public System
    {
        void setup_reactive_systems(entt::registry&) override;
        void run(entt::registry&) override;
        void prepare_to_paint(entt::registry&) override;
        ~Knob();
    private:
        entt::observer dragged;
        entt::observer new_knobs;
        entt::observer updated_knobs;
    };
}
