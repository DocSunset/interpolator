#include "vis.h"
#include <iostream>
#include "components/window.h"
#include "components/pca.h"
#include "color_spaces.h"
#include <simple/boundaries.h>

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
                { 0.7*window.w/2 * pos.x()
                , 0.8*window.h/2 * pos.y()
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
        const auto& pca = registry.ctx<Component::DestinationPCA>();
        auto col = pca.projection * (destination - pca.mean);
        auto h = 360 * Simple::clip(col.x() * 0.5 + 0.5);
        auto s = Simple::clip(col.y() * 0.5 + 0.5);
        auto l = Simple::clip(col.z() * 0.5 + 0.5);
        Component::Color color =
                { h
                , std::sqrt(s)
                , l
                , 1.0f
                };
        color = System::hsla2rgba(color);
        return color;
    }
}
