#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class Platform : public System
    {
        class PlatformImplementation;
        PlatformImplementation* pimpl;
    public:
        Platform(entt::registry&);
        ~Platform();
        void run(entt::registry&) override;
    };

}
