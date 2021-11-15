#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class LinePainter : public System
    {
        class Implementation;
        Implementation * pimpl;
    public:
        LinePainter();
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        ~LinePainter();
        void run(entt::registry&) override;
    };
}
