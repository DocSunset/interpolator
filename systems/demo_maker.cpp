#include "demo_maker.h"
#include "components/demo.h"
#include "components/color.h"
#include "components/position.h"
#include "components/draggable.h"

namespace System
{
    DemoMaker::DemoMaker(entt::registry& registry)
    {
        using Component::Demo;
        using Component::Color;
        using Component::Position;
        using Component::Selected;
        using Component::SelectionHovered;
        using Component::Draggable;

        for (int i = 0; i < 5; ++i)
        {
            auto entity = registry.create();
            registry.emplace<Demo>(entity, i);
            registry.emplace<Color>(entity, Color::Random());
            registry.emplace<Position>(entity, Position::Random());
            registry.emplace<Selected>(entity, false);
            registry.emplace<SelectionHovered>(entity, false);
            registry.emplace<Draggable>(entity
                    , 25
                    , Position::Zero()
                    , Position::Zero()
                    , Position::Zero()
                    , Position::Zero()
                    );
        }
    }

    void DemoMaker::run(entt::registry& registry)
    {
    }
}
