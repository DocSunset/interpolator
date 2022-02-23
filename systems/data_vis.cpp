#include "data_vis.h"
#include "components/vis.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/color.h"
#include "components/window.h"
#include "components/manual_vis.h"
#include "common/vis.h"

namespace
{
    struct BackupPosition
    {
        Component::Position value;
    };

    struct BackupColor
    {
        Component::Color value;
    };

    void update_source(entt::registry& registry, entt::entity entity)
    {
        if (registry.ctx<Component::ManualVis>()) return;
        registry.replace<Component::Position>(entity
                , System::source_to_position(registry, entity)
                );
    }

    void update_sources(entt::registry& registry)
    {
        for (auto entity : registry.view<Component::Demo>())
             update_source(registry, entity);
    }

    void update_destination(entt::registry& registry, entt::entity entity)
    {
        if (registry.ctx<Component::ManualVis>()) return;
        registry.replace<Component::Color>(entity
                , System::destination_to_color(registry, entity)
                );
    }

    void update_destinations(entt::registry& registry)
    {
        for (auto entity : registry.view<Component::Demo>())
             update_destination(registry, entity);
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
        auto p = registry.get<Component::Position>(entity);
        registry.emplace_or_replace<BackupPosition>(entity, p);
    }

    void track_color(entt::registry& registry, entt::entity entity)
    {
        if (not registry.all_of<Component::Demo>(entity)) return;
        auto c = registry.get<Component::Color>(entity);
        registry.emplace_or_replace<BackupColor>(entity, c);
    }

    void restore_backups(entt::registry& registry)
    {
        for (auto entity : registry.view<Component::Demo>())
        {
            if (registry.all_of<BackupPosition>(entity))
            {
                auto p = registry.get<BackupPosition>(entity).value;
                registry.replace<Component::Position>(entity, p);
            }
            if (registry.all_of<BackupColor>(entity))
            {
                auto c = registry.get<BackupColor>(entity).value;
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

            restore_backups(registry);

            registry.on_construct<Component::Position>().connect<&track_position>();
            registry.on_update<Component::Position>().connect<&track_position>();
            registry.on_construct<Component::Color>().connect<&track_color>();
            registry.on_update<Component::Color>().connect<&track_color>();
        }
        else
        {
            registry.on_update<Component::Position>().disconnect<&track_position>();
            registry.on_update<Component::Color>().disconnect<&track_color>();
            registry.on_construct<Component::Vis>().connect<&on_construct>();
            registry.on_update<Component::Demo::Source>().connect<&update_source>();
            registry.on_update<Component::Demo::Destination>().connect<&update_destination>();
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
        registry.on_construct<Component::ManualVis>().connect<&on_mode_switch>();
        registry.on_update<Component::ManualVis>().connect<&on_mode_switch>();
    }

    void DataVis::run(entt::registry& registry)
    {
    }
}
