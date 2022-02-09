#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class Demonstration : public System
    {
        entt::observer dragged;
        entt::observer updated_demos;
    public:
        void setup_reactive_systems(entt::registry&) override;
        void run(entt::registry&) override;
        void prepare_to_paint(entt::registry&) override;
        ~Demonstration();
    };
}
