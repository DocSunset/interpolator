#include "vis.h"
#include "components/window.h"

namespace System
{
    Component::Position source_to_position(const entt::registry& registry, entt::entity entity)
    {
        const auto& source = registry.get<Component::Demo::Source>(entity);
        return source_to_position(registry, source);
    }

    Component::Position source_to_position(const entt::registry& registry
            , const Component::Demo::Source& source
            )
    {
        const auto& window = registry.ctx<Component::Window>();

        return { window.w * (source[0] - 0.5f)
               , window.h * (source[1] - 0.5f)
               };
    }

    Component::Demo::Source position_to_source(const entt::registry& registry
            , const Component::Position& position
            )
    {
        const auto& window = registry.ctx<Component::Window>();
        return { 0.5f + (position.x / window.w)
               , 0.5f + (position.y / window.h)
               };
    }

    Component::Color destination_to_color(const entt::registry& registry, entt::entity entity)
    {
        const auto& destination = registry.get<Component::Demo::Destination>(entity);
        return destination_to_color(registry, destination);
    }

    Component::Color destination_to_color(const entt::registry& registry
            , const Component::Demo::Destination& destination
            )
    {
        return { destination[0]
               , destination[1]
               , destination[2]
               , 1.0f
               };
    }
}
