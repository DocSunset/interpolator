#include "demo_vis.h"
#include <iostream>
#include "components/demo.h"
#include "components/position.h"
#include "components/color.h"
#include "components/window.h"
#include "common/vis.h"

namespace
{
    void update_source(entt::registry& registry, entt::entity entity)
    {
        registry.emplace_or_replace<Component::Position>(entity
                , System::source_to_position(registry, entity)
                );
    }

    void update_destination(entt::registry& registry, entt::entity entity)
    {
        registry.emplace_or_replace<Component::Color>(entity
                , System::destination_to_color(registry, entity)
                );
    }

    void on_construct(entt::registry& registry, entt::entity entity)
    {
        update_source(registry, entity);
        update_destination(registry, entity);
    }
}

namespace System
{
    void DemoVis::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_construct<Component::Demo>().connect<&on_construct>();
        registry.on_update<Component::Demo::Source>().connect<&update_source>();
        registry.on_update<Component::Demo::Destination>().connect<&update_destination>();
        updated_positions.connect(registry, entt::collector
                .update<Component::Position>().where<Component::Demo>()
                );
    }

    void DemoVis::run(entt::registry& registry)
    {
        auto window = registry.ctx<Component::Window>();
        updated_positions.each([&](auto entity)
        {
            auto& source = registry.get<Component::Demo::Source>(entity);
            const auto& position = registry.get<Component::Position>(entity);

            // we assign here instead of using registry.replace
            // to ensure that we don't trigger an update event that would cause
            // the position to be updated again...
            source = position_to_source(registry, position);
        });
    }
}
