#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class DemoMaker : public System
    {
    public:
        DemoMaker(entt::registry&);
        void run(entt::registry&) override;
    };
}
