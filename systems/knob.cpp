#include "knob.h"
#include "components/color.h"
#include "components/draggable.h"
#include "components/demo.h"
#include "components/knob.h"
#include "components/window.h"
#include "components/paint_flag.h"
#include "components/position.h"
#include "components/circle.h"
#include "systems/common/draggable.h"
#include "systems/common/vis.h"
#include "shader/knob_viewer.h"
#include <simple/constants/pi.h>
#include <simple/boundaries.h>
#include <iostream>

namespace
{
    // interaction

    void position_knob(entt::registry& registry, entt::entity entity, const Component::Window& window)
    {
        constexpr float padding = 5;
        auto knob = registry.get<Component::Knob>(entity);
        auto radius = padding + knob.radius;
        float top_left_x = (window.w / 2.0f) - radius;
        float top_left_y = (window.h / 2.0f) - radius;

        registry.replace<Component::Position>(entity, top_left_x, top_left_y - (knob.index * radius));
    }

    void on_window_update(entt::registry& registry, entt::entity entity)
    {
        const auto& window = registry.ctx<Component::Window>();
        auto knobs = registry.view<Component::Knob>();
        for (auto knob : knobs) position_knob(registry, knob, window);
    }

    void sync_knob_values(entt::registry& registry)
    {
        auto knobs = registry.view<Component::Knob>();
        auto selected_demos = registry.view<Component::Demo, Component::Selected>();
        Component::Color color{0.0f,0.0f,0.0f,0.0f};

        int n_demos = 0;
        for (auto demo : selected_demos) ++n_demos;
        if (n_demos == 0)
        {
            auto p = registry.ctx<Component::Demo::Destination>();
            color = System::destination_to_color(registry, p);
            for (auto knob_entity : knobs)
            {
                auto knob = registry.get<Component::Knob>(knob_entity);
                registry.patch<Component::Knob>(knob_entity
                        , [&](auto& k) { k.value = p[k.index]; }
                        );
                registry.replace<Component::Color>(knob_entity, color);
            }
        }
        else
        {
            Component::Demo::Destination p = Component::Demo::Destination::Zero();
            for (auto demo : selected_demos)
            {
                auto dcolor = registry.get<Component::Color>(demo);
                p += (1.0 / n_demos) * (registry.get<Component::Demo::Destination>(demo) - p);
            }
            color = System::destination_to_color(registry, p);
            for (auto knob_entity : knobs)
            {
                n_demos = 0;
                auto knob = registry.get<Component::Knob>(knob_entity);
                registry.patch<Component::Knob>(knob_entity
                        , [&](auto& knob) { knob.value = p[knob.index]; }
                        );
                registry.replace<Component::Color>(knob_entity, color);
            }
        }
    }

    void drag_knobs(entt::registry& registry, entt::observer& dragged)
    {
        if (dragged.empty()) return;
        const auto& win = registry.ctx<Component::Window>();
        dragged.each([&](const auto entity)
        {
            auto& drag = registry.get<Component::Draggable>(entity);
            auto& knob = registry.get<Component::Knob>(entity);

            float delta = drag.delta.y / win.h;
            registry.patch<Component::Knob>(entity
                    , [&](auto& knob) { knob.value = Simple::clip(knob.value + delta); }
                    );
            drag.delta = {0,0};

            std::size_t num_selected_demos = 0;
            auto view = registry.view<Component::Selected, Component::Demo>();
            for (auto entity : view)
            {
                auto& p = registry.get<Component::Demo::Destination>(entity);
                p[knob.index] = Simple::clip(p[knob.index] + delta);
                ++num_selected_demos;
            }
            if (num_selected_demos == 0)
            {
                auto& p = registry.ctx<Component::Demo::Destination>();
                p[knob.index] = Simple::clip(p[knob.index] + delta);
            }
        });
        auto view = registry.view<Component::Selected, Component::Demo>();
        for (auto entity : view)
        {
            // trigger update events
            registry.patch<Component::Demo::Destination>(entity, [&](auto& p){return;});
        }
    }

    // view

    struct KnobView
    {
        entt::entity background;
        entt::entity indicator;
    };

    Component::Position get_indicator_position(const entt::registry& registry, entt::entity entity)
    {
        constexpr float deg_to_rad = Simple::pi / 180.0f;
        const auto& knob = registry.get<Component::Knob>(entity);
        auto radius = knob.radius/2.0f - 15;
        auto center = registry.get<Component::Position>(entity);
        auto angle = (-120.0f - knob.value * 300.0f) * deg_to_rad;
        angle = Simple::wrap(angle, -Simple::pi, Simple::pi);
        return {center.x + radius * std::cos(angle), center.y + radius * std::sin(angle)};
    }

    void update_knobview(entt::registry& registry, entt::entity entity)
    {
        auto knobview = registry.get<KnobView>(entity);
        auto ring_color = System::hover_select_color(registry, entity);
        auto fill_color = registry.get<Component::Color>(entity);
        auto position = registry.get<Component::Position>(entity);
        auto radius = registry.get<Component::Knob>(entity).radius;
        auto indicator_position = get_indicator_position(registry, entity);

        auto emp_or_rep = [&](auto entity, auto radius, auto position, auto color, auto border, auto border_thick)
        {
            registry.emplace_or_replace<Component::Circle>(entity,
                    Component::Circle
                    { {color[0], color[1], color[2], color[3]}
                    , {border[0], border[1], border[2], border[3]}
                    , {position.x, position.y}
                    , radius
                    , border_thick
                    });
        };

        emp_or_rep(knobview.background, radius, position, ring_color, fill_color, 5.0f);
        emp_or_rep(knobview.indicator, 15.0f, indicator_position, fill_color, Component::Color{0,0,0,1}, 3.0f);
    }

    void destroy_knobview(entt::registry& registry, entt::entity knob)
    {
        auto& knobview = registry.get<KnobView>(knob);
        registry.destroy(knobview.background);
        registry.destroy(knobview.indicator);
    }

    void construct_knob(entt::registry& registry, entt::entity entity)
    {
        const auto& knob = registry.get<Component::Knob>(entity);
        registry.emplace<Component::Position>(entity);
        registry.emplace<Component::Color>(entity);
        registry.emplace<Component::Draggable>(entity, knob.radius);

        position_knob(registry, entity, registry.ctx<Component::Window>());

        registry.emplace<KnobView>(entity, registry.create(), registry.create());
        update_knobview(registry, entity);
    }
}

namespace System
{
    void Knob::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_construct<Component::Knob>().connect<&construct_knob>();
        registry.on_destroy<Component::Knob>().connect<&destroy_knobview>();
        registry.on_update<Component::Window>().connect<&on_window_update>();

        dragged.connect(registry, entt::collector
                .update<Component::Draggable>()
                .where<Component::Knob>()
                );

        updated_knobs.connect(registry, entt::collector
                .update<Component::Position>().where<Component::Knob>()
                .update<Component::Knob>()
                );
    }

    void Knob::prepare_registry(entt::registry& registry)
    {
        for (int i = 0; i < Component::Demo::num_destinations; ++i)
        {
            auto knob = registry.create();
            registry.emplace<Component::Knob>(knob, i, 0.0f, 75.0f);
        }
    }

    void Knob::prepare_to_paint(entt::registry& registry)
    {
        sync_knob_values(registry);
        drag_knobs(registry, dragged);
        updated_knobs.each([&](auto entity){update_knobview(registry, entity);});
    }

    Knob::~Knob() {}
}
