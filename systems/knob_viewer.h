#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    struct KnobViewer : public System
    {
        void setup_reactive_systems(entt::registry&) override;
        void prepare_to_paint(entt::registry&) override;
        ~KnobViewer();
    private:
        entt::observer new_knobs;
        entt::observer updated_knobs;
    };
}
