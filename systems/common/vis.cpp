#include "vis.h"
#include <iostream>
#include "components/window.h"
#include "components/pca.h"

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
        const auto& pca = registry.ctx<Component::SourcePCA>();
        Eigen::Vector2d pos = pca.projection * (source - pca.mean);
        Component::Position position = 
                { window.w * pos.x()
                , window.h * pos.y()
                };
        return position;
    }

    Component::Demo::Source position_to_source(const entt::registry& registry
            , const Component::Position& position
            )
    {
        const auto& window = registry.ctx<Component::Window>();
        const auto& pca = registry.ctx<Component::SourcePCA>();
        Eigen::Vector2d src2;
        src2 <<  position.x / window.w
               , position.y / window.h;
        Component::Demo::Source source = (pca.inverse_projection * src2) + pca.mean;
        return source;
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
