#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{

    class PlatformImplementation;
    
    class Platform : public System
    {
        PlatformImplementation* pimpl;
    public:
        Platform(entt::registry&);
        ~Platform();
        void run(entt::registry&) override;
    };

}
