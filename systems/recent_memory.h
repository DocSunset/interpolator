#pragma once
#include "system.h"

namespace System
{
    struct RecentMemory : public System
    {
        // void construct_system() override;
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        // void run(entt::registry&) override;
        void prepare_to_paint(entt::registry&) override;
        // void paint(entt::registry&) override;
        ~RecentMemory() = default;
    };
}
