#pragma once

#include "system.h"

namespace System
{
    class Libmapper : public System
    {
        struct Implementation;
        Implementation * pimpl;
    public:
        void construct_system() override;
        // void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        void run(entt::registry&) override;
        // void prepare_to_paint(entt::registry&) override;
        // void paint(entt::registry&) override;
        ~Libmapper();
    };
}
