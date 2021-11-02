#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class InterpolatorVisualizer : public System
    {
        class Implementation;
        Implementation * pimpl;
    public:
        InterpolatorVisualizer();
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        ~InterpolatorVisualizer();
        void run(entt::registry&) override;
    };
}
