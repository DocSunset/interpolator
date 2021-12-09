#include "button.h"
#include "components/button.h"
#include "components/position.h"
#include "components/color.h"
#include "components/mouse_button.h"
#include "components/circle.h"
#include <iostream>

namespace
{
    struct ButtonDown {};

    void maybe_down_button(entt::registry& registry, Component::Position down_position, bool& consumed)
    {
        float closest = std::numeric_limits<float>::max();
        entt::entity downed = entt::null;
        auto view = registry.view<Component::Button>();
        for (auto e : view)
        {
            auto position = registry.get_or_emplace<Component::Position>(e, Component::Position::Zero());
            auto btn = view.get<Component::Button>(e);
            auto dist = distance(down_position, position); 
            if (dist <= btn.radius && dist < closest)
            {
                downed = e;
                closest = dist;
            }
        }
        if (downed == entt::null) return;
        registry.emplace<ButtonDown>(downed);
        consumed = true;
    }

    void maybe_press_button(entt::registry& registry, Component::Position up_position, bool& consumed)
    {
        for (auto e : registry.view<ButtonDown>())
        {
            auto position = registry.get<Component::Position>(e);
            auto btn = registry.get<Component::Button>(e);
            registry.erase<ButtonDown>(e);
            if (not (distance(up_position, position) <= btn.radius)) continue;
            registry.emplace<Component::ButtonPress>(e);
            consumed = true;
        }
    }

    void on_left_mouse_button(entt::registry& registry, entt::entity entity)
    {
        auto& mbtn = registry.get<Component::LeftMouseButton>(entity);
        if (mbtn.pressed) maybe_down_button(registry, mbtn.down_position, mbtn.consumed);
        else maybe_press_button(registry, mbtn.up_position, mbtn.consumed);
        for (auto e : registry.view<Component::ButtonPress>())
            registry.erase<Component::ButtonPress>(e);
    }
}

namespace System
{
    void Button::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_update<Component::LeftMouseButton>().connect<&on_left_mouse_button>();
    }

    void Button::prepare_to_paint(entt::registry& registry)
    {
        using namespace Component;
        auto view = registry.view<Component::Button>();
        for (auto e : view)
        {
            auto position = registry.get_or_emplace<Position>(e, Position::Zero());
            auto color = registry.get_or_emplace<Color>(e, Color{1,1,1,1});
            auto radius = view.get<Component::Button>(e).radius;
            registry.emplace_or_replace<Circle>(e, Circle
                    { .color = {color.r, color.g, color.b, color.a}
                    , .border = {color.r, color.g, color.b, color.a}
                    , .position = {position.x, position.y}
                    , .radius = radius
                    , .border_thickness = 0
                    });
        }
    }
}
