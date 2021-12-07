#include "draggable.h"
#include "components/position.h"
#include "components/draggable.h"

namespace System
{
    void drag_update_position(entt::registry& registry, entt::observer& observer)
    {
        observer.each([&](const auto entity)
        {
            auto& p    = registry.get<Component::Position>(entity);
            auto& drag = registry.get<Component::Draggable>(entity);
            registry.replace<Component::Position>(entity, p + drag.delta);
            drag.delta = {0, 0};
        });
    }

    Component::Color hover_select_color(entt::registry& registry, entt::entity entity
            , Component::Color normal
            , Component::Color hover
            , Component::Color select
            )
    {
        auto s = registry.get<Component::Selectable>(entity);
        auto h = registry.get<Component::SelectionHovered>(entity);
        if (s) return select;
        else if (h) return hover;
        else return normal;
    }
}
