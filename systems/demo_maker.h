#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class DemoMaker : public System
    {
    public:
        DemoMaker();
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        void run(entt::registry&) override;
    };
}
