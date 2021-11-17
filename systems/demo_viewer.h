#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    struct DemoViewer : public System
    {
        void setup_reactive_systems(entt::registry&) override;
        void run(entt::registry&) override;
        ~DemoViewer();
    private:
        entt::observer updated_demos;
        entt::observer new_demos;
    };
}
