#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class DemoViewer : public System
    {
        class Implementation;
        Implementation * pimpl;
    public:
        DemoViewer();
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        ~DemoViewer();
        void run(entt::registry&) override;
    };
}
