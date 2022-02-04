#pragma once

#include "system.h"

namespace System
{
    class Interpolator : public System
    {
        class Implementation;
        Implementation * pimpl;
    public:
        void construct_system() override;
        void setup_reactive_systems(entt::registry&) override;
        void run(entt::registry&) override;
        ~Interpolator();
    };
}
