#include "smooth_position.h"

#include <tuple>
#include "components/smooth_position.h"
#include "components/paint_flag.h"

namespace
{
    struct SmoothingStatus {float diff;};

    void on_construct(entt::registry& registry, entt::entity entity)
    {
        const auto& sp = registry.get<Component::SmoothPosition>(entity);
        registry.emplace_or_replace<Component::Position>(entity, sp.x, sp.y);
    }

    auto update_status(entt::registry& registry, entt::entity entity)
    {
        const auto& current_position = registry.get<Component::Position>(entity);
        const auto& target_position = registry.get<Component::SmoothPosition>(entity);
        const auto& smoothing = registry.emplace_or_replace<SmoothingStatus>(entity, norm_squared(current_position - target_position));
        return std::make_tuple(smoothing, current_position, target_position);
    }

    void smooth(entt::registry& registry, entt::entity entity)
    {
        constexpr float s = 0.3;
        const auto & [smoothing, current_position, target_position] = update_status(registry, entity);

        if (smoothing.diff > 0.001) registry.patch<Component::Position>(entity, [&](auto& position)
        {
            position.x = s * target_position.x + (1.0f - s) * current_position.x;
            position.y = s * target_position.y + (1.0f - s) * current_position.y;
        });
        else
        {
            registry.replace<Component::Position>(entity, target_position);
            registry.remove<SmoothingStatus>(entity);
        }

    }
}

namespace System
{
    void SmoothPosition::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_construct<Component::SmoothPosition>().connect<&on_construct>();
        registry.on_update<Component::SmoothPosition>().connect<&update_status>();
    }

    void SmoothPosition::run(entt::registry& registry)
    {
        if (registry.view<SmoothingStatus>().size()) registry.ctx<Component::PaintFlag>().set();
    }

    void SmoothPosition::prepare_to_paint(entt::registry& registry)
    {
        for (auto entity : registry.view<SmoothingStatus>())
            smooth(registry, entity);
    }
}
