#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class Glyph : public System
    {
        class Implementation;
        Implementation * pimpl;
    public:
        Glyph();
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        ~Glyph();
        void run(entt::registry&) override;
    };
}
