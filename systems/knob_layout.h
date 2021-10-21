#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class KnobLayout : public System
    {
        class Implementation;
        Implementation * pimpl;
    public:
        KnobLayout();
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        ~KnobLayout();
        void run(entt::registry&) override;
    };
}
