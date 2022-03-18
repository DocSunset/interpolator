#include "data_vis.h"
#include "components/vis.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/color.h"
#include "components/window.h"
#include "components/pca.h"
#include "components/manual_vis.h"
#include "components/smooth_position.h"
#include "common/vis.h"

namespace
{
    void update_source(entt::registry& registry, entt::entity entity)
    {
        if (registry.ctx<Component::ManualVis>()) return;
        if (not registry.all_of<Component::Vis>(entity)) return;
        if (registry.all_of<Component::Demo>(entity) && registry.get<Component::Demo>(entity).destroyed) return;
        registry.replace<Component::SmoothPosition>(entity
                , System::source_to_position(registry, entity)
                );
    }

    void update_sources(entt::registry& registry)
    {
        for (auto entity : registry.view<Component::Vis>())
             update_source(registry, entity);
    }

    void update_destination(entt::registry& registry, entt::entity entity)
    {
        if (registry.ctx<Component::ManualVis>()) return;
        if (not registry.all_of<Component::Vis>(entity)) return;
        registry.replace<Component::Color>(entity
                , System::destination_to_color(registry, entity)
                );
    }

    void update_destinations(entt::registry& registry)
    {
        for (auto entity : registry.view<Component::Vis>())
             update_destination(registry, entity);
    }

    void on_source_update(entt::registry& registry, entt::entity entity)
    {
        update_sources(registry);
    }

    void on_destination_update(entt::registry& registry, entt::entity _)
    {
        update_destinations(registry);
    }

    void on_construct(entt::registry& registry, entt::entity entity)
    {
        update_source(registry, entity);
        update_destination(registry, entity);
    }

    void on_window(entt::registry& registry, entt::entity entity)
    {
        if (registry.ctx<Component::ManualVis>())
        {
            // arguably we should do something to scale the positions to match the new window size
        }
        else 
        {
            update_sources(registry);
        }
    }

    void track_position(entt::registry& registry, entt::entity entity)
    {
        if (not registry.all_of<Component::Demo>(entity)) return;
        auto p = registry.get<Component::ManualPosition>(entity);
        registry.emplace_or_replace<Component::Position>(entity, p.value);
    }

    void track_color(entt::registry& registry, entt::entity entity)
    {
        if (not registry.all_of<Component::Demo>(entity)) return;
        auto c = registry.get<Component::ManualColor>(entity);
        registry.emplace_or_replace<Component::Color>(entity, c.value);
    }

    void restore_backups(entt::registry& registry)
    {
        for (auto entity : registry.view<Component::Demo>())
        {
            if (registry.all_of<Component::ManualPosition>(entity))
            {
                auto p = registry.get<Component::ManualPosition>(entity).value;
                registry.replace<Component::Position>(entity, p);
            }
            if (registry.all_of<Component::ManualColor>(entity))
            {
                auto c = registry.get<Component::ManualColor>(entity).value;
                registry.replace<Component::Color>(entity, c);
            }
        }
    }

    void on_mode_switch(entt::registry& registry, entt::entity entity)
    {
        if (registry.ctx<Component::ManualVis>())
        {
            registry.on_construct<Component::Vis>().disconnect<&on_construct>();
            registry.on_update<Component::Demo::Source>().disconnect<&update_source>();
            registry.on_update<Component::Demo::Destination>().disconnect<&update_destination>();
            registry.on_update<Component::SourcePCA>().disconnect<&on_source_update>();
            registry.on_update<Component::DestinationPCA>().disconnect<&on_destination_update>();

            restore_backups(registry);

            registry.on_update<Component::ManualPosition>().connect<&track_position>();
            registry.on_update<Component::ManualColor>().connect<&track_color>();
        }
        else
        {
            registry.on_update<Component::ManualPosition>().disconnect<&track_position>();
            registry.on_update<Component::ManualColor>().disconnect<&track_color>();

            registry.on_construct<Component::Vis>().connect<&on_construct>();
            registry.on_update<Component::Demo::Source>().connect<&update_source>();
            registry.on_update<Component::Demo::Destination>().connect<&update_destination>();
            registry.on_update<Component::SourcePCA>().connect<&on_source_update>();
            registry.on_update<Component::DestinationPCA>().connect<&on_destination_update>();
            update_sources(registry);
            update_destinations(registry);
        }
    }
}

namespace System
{
    void DataVis::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_update<Component::Window>().connect<&on_window>();
        registry.on_construct<Component::ManualPosition>().connect<&track_position>();
        registry.on_construct<Component::ManualColor>().connect<&track_color>();
        registry.on_construct<Component::ManualVis>().connect<&on_mode_switch>();
        registry.on_update<Component::ManualVis>().connect<&on_mode_switch>();
    }

    void DataVis::run(entt::registry& registry)
    {
    }
}
