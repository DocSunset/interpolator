#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    struct DemoMaker : public System
    {
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        ~DemoMaker();
    };
}
