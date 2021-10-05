#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class Platform : public System
    {
        class Implementation;
        Implementation* pimpl;
    public:
        Platform();
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        ~Platform();
        void run(entt::registry&) override;
        void * window() const;
    };
}
