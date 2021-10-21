#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class Knob : public System
    {
        class Implementation;
        Implementation * pimpl;
    public:
        Knob();
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        ~Knob();
        void run(entt::registry&) override;
    };
}
