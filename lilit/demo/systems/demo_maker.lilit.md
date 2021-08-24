# Creating Demonstrations

This is a non-functional stub for now that just adds a handful of demo entities
when constructed and has no real functionality.

```cpp
// @#'demo/interpolators/systems/demo_maker.h'
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
// @/

// @#'demo/interpolators/systems/demo_maker.cpp'
#include "demo_maker.h"
#include "components/demo.h"

namespace System
{
    DemoMaker::DemoMaker(entt::registry& registry)
    {
        using Component::Demo;
        for (int i = 0; i < 5; ++i)
        {
            auto entity = registry.create();
            auto& d = registry.emplace<Demo>(entity);
            d = Demo::Random();
        }
    }
}
// @/
```
