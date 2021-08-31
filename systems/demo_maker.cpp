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

    void DemoMaker::run(entt::registry& registry)
    {
    }
}
