#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class DemoDragger : public System
    {
        class Implementation;
        Implementation * pimpl;
    public:
        DemoDragger();
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        ~DemoDragger();
        void run(entt::registry&) override;
    };
}
