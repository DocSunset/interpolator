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

    void maybe_down_button(entt::registry& registry, Component::Position down_position)
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
    }

    void maybe_press_button(entt::registry& registry, Component::Position up_position)
    {
        for (auto e : registry.view<ButtonDown>())
        {
            auto position = registry.get<Component::Position>(e);
            auto btn = registry.get<Component::Button>(e);
            if (distance(up_position, position) <= btn.radius)
                registry.emplace<Component::ButtonPress>(e);
            registry.erase<ButtonDown>(e);
        }
    }

    void on_left_mouse_button(entt::registry& registry, entt::entity entity)
    {
        const auto& mbtn = registry.get<Component::LeftMouseButton>(entity);
        if (mbtn.pressed) maybe_down_button(registry, mbtn.down_position);
        else maybe_press_button(registry, mbtn.up_position);
        for (auto e : registry.view<Component::ButtonPress>())
            registry.erase<Component::ButtonPress>(e);
    }

    void test(entt::registry& registry, entt::entity entity)
    {
        std::cout << "button press!\n";
    }
}

namespace System
{
    void Button::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_update<Component::LeftMouseButton>().connect<&on_left_mouse_button>();
        registry.on_construct<Component::ButtonPress>().connect<&test>();
    }

    void Button::prepare_registry(entt::registry& registry)
    {
        auto e = registry.create();
        registry.emplace<Component::Button>(e, 50);
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
                    { .color = {color.r, color.g, color.g, color.a}
                    , .border = {color.r, color.g, color.g, color.a}
                    , .position = {position.x, position.y}
                    , .radius = radius
                    , .border_thickness = 0
                    });
        }
    }
}
