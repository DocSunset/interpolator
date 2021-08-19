#pragma once

#include <entt/entt.hpp>
#include "../system.h"

namespace System
{

    class PlatformImplementation;
    
    class Platform : public System
    {
        PlatformImplementation* pimpl;
    public:
        Platform();
        ~Platform();
        void run(entt::registry&) override;
    };

}
