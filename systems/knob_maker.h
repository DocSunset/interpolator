#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class KnobMaker : public System
    {
        class Implementation;
        Implementation * pimpl;
    public:
        KnobMaker();
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        ~KnobMaker();
        void run(entt::registry&) override;
    };
}
