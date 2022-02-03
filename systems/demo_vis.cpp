#include "demo_vis.h"
#include <iostream>
#include "components/demo.h"
#include "components/position.h"
#include "components/color.h"
#include "components/window.h"
#include "common/vis.h"

namespace
{
    void update_demo(entt::registry& registry, entt::entity entity)
    {
        registry.emplace_or_replace<Component::Position>(entity
                , System::source_to_position(registry, entity)
                );

        registry.emplace_or_replace<Component::Color>(entity
                , System::destination_to_color(registry, entity)
                );
    }
}

namespace System
{
    void DemoVis::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_construct<Component::Demo>().connect<&update_demo>();
        registry.on_update<Component::Demo>().connect<&update_demo>();
        updated_demos.connect(registry, entt::collector
                .update<Component::Position>().where<Component::Demo>()
                );
    }

    void DemoVis::run(entt::registry& registry)
    {
        auto window = registry.ctx<Component::Window>();
        auto f = [&](auto entity)
        {
            auto& demo = registry.get<Component::Demo>(entity);
            const auto& position = registry.get<Component::Position>(entity);

            // we assign here instead of using registry.replace
            // to ensure that we don't trigger an update event that would cause
            // the position to be updated again...
            demo.source = position_to_source(registry, position);
        };
        updated_demos.each(f);
    }
}
