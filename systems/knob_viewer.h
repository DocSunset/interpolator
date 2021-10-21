#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class KnobViewer : public System
    {
        class Implementation;
        Implementation * pimpl;
    public:
        KnobViewer();
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        ~KnobViewer();
        void run(entt::registry&) override;
    };
}
