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
        bool testing = false;
        void construct_system() override;
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        ~Platform();
        void run(entt::registry&) override;
        void paint(entt::registry&) override;
        void swap_window() const;
    };
}
