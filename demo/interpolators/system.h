#pragma once

#include <entt/entt.hpp>

namespace System
{
    class System
    {
    public:
        virtual void run(entt::registry&) = 0;
    };
}
