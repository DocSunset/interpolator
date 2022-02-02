#include "demo_vis.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/color.h"
#include "components/window.h"

namespace
{
    void update_demo(entt::registry& registry, entt::entity entity)
    {
        auto& demo = registry.get<Component::Demo>(entity);
        auto window = registry.ctx<Component::Window>();

        registry.emplace_or_replace<Component::Position>(entity
                , window.w * (demo.source[0] * 2 - 1)
                , window.h * (demo.source[1] * 2 - 1)
                );

        registry.emplace_or_replace<Component::Color>(entity
                , demo.destination[0]
                , demo.destination[1]
                , demo.destination[2]
                , 1.0f
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
        auto f = [&](auto entity)
        {
            auto& demo = registry.get<Component::Demo>(entity);
            const auto& position = registry.get<Component::Position>(entity);

            // we assign here instead of using registry.replace
            // to ensure that we don't trigger an update event that would cause
            // the position to be updated again...
            demo.source[0] = position.x;
            demo.source[1] = position.y;
        };
        updated_demos.each(f);
    }
}
