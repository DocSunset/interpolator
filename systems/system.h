#pragma once

#include <entt/entt.hpp>

namespace System
{
    class System
    {
    public:
        virtual ~System() = default;
        virtual void run(entt::registry&) = 0;
    };
}
