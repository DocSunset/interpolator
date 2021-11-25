#pragma once
#include "system.h"

namespace System
{
    class TextPainter : public System
    {
        class Implementation;
        Implementation * pimpl;
    public:
        void construct_system() override;
        void setup_reactive_systems(entt::registry&) override;
        void prepare_registry(entt::registry&) override;
        void prepare_to_paint(entt::registry&) override;
        void paint(entt::registry&) override;
        ~TextPainter();
    };
}
