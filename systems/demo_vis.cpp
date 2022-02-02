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
        registry.emplace_or_replace<Component::Position>(entity, window.w * (demo.source[0] * 2 - 1), window.h * (demo.source[1] * 2 - 1));
        registry.emplace_or_replace<Component::Color>(entity, demo.destination[0], demo.destination[1], demo.destination[2], 1.0f);
    }
}

namespace System
{
    void DemoVis::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_construct<Component::Demo>().connect<&update_demo>();
        registry.on_update<Component::Demo>().connect<&update_demo>();
    }
}
