#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{
    class DemoMaker : public System
    {
        DemoMaker(entt::registry&);
    };
}
