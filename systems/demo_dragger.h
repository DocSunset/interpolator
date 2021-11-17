#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    struct DemoDragger : public System
    {
        void setup_reactive_systems(entt::registry&) override;
        void run(entt::registry&) override;
        ~DemoDragger();
    private:
        entt::observer dragged;
    };
}
