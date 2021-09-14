#include "demo_maker.h"
#include "components/demo.h"
#include "components/color.h"
#include "components/position.h"

namespace System
{
    DemoMaker::DemoMaker(entt::registry& registry)
    {
        using Component::Demo;
        using Component::Color;
        using Component::Position;
        for (int i = 0; i < 5; ++i)
        {
            auto entity = registry.create();
            registry.emplace<Demo>(entity, i);
            registry.emplace<Color>(entity, Color::Random());
            registry.emplace<Position>(entity, Position::Random());
        }
    }

    void DemoMaker::run(entt::registry& registry)
    {
    }
}
