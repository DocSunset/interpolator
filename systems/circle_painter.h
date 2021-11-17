#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    struct CirclePainter : public System
    {
        void setup_reactive_systems(entt::registry&) override;
        ~CirclePainter();
    };
}
