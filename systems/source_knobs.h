#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    struct SourceKnobs : public System
    {
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        void prepare_to_paint(entt::registry&) override;
        ~SourceKnobs() = default;
    };
}
