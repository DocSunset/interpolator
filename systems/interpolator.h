#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class Interpolator : public System
    {
        class Implementation;
        Implementation * pimpl;
    public:
        Interpolator();
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        ~Interpolator();
        void run(entt::registry&) override;
    };
}
