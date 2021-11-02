#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class CirclePainter : public System
    {
        class Implementation;
        Implementation * pimpl;
    public:
        CirclePainter();
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        ~CirclePainter();
        void run(entt::registry&) override;
    };
}
