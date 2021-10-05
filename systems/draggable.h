#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class Draggable : public System
    {
        struct Implementation;
        Implementation * pimpl;
    public:
        Draggable();
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        ~Draggable();
        void run(entt::registry&) override;
    };
}
