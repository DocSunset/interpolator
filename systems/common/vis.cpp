#include "vis.h"
#include "components/window.h"

namespace System
{
    Component::Position source_to_position(const entt::registry& registry, entt::entity entity)
    {
        const auto& demo = registry.get<Component::Demo>(entity);
        const auto& window = registry.ctx<Component::Window>();

        return { window.w * (demo.source[0] - 0.5f)
               , window.h * (demo.source[1] - 0.5f)
               };
    }

    Component::Demo::Source position_to_source(const entt::registry& registry, const Component::Position& position)
    {
        const auto& window = registry.ctx<Component::Window>();
        return { 0.5f + (position.x / window.w)
               , 0.5f + (position.y / window.h)
               };
    }

    Component::Color destination_to_color(const entt::registry& registry, entt::entity entity)
    {
        const auto& demo = registry.get<Component::Demo>(entity);
        return { demo.destination[0]
               , demo.destination[1]
               , demo.destination[2]
               , 1.0f
               };
    }
}
