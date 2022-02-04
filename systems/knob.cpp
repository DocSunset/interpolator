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
#include <functional>
#include <vector>
#include <numeric>

namespace
{
    // interaction

    void position_knob_w(entt::registry& registry, entt::entity entity, const Component::Window& window)
    {
        constexpr float padding = 5;
        auto knob = registry.get<Component::Knob>(entity);
        auto radius = padding + registry.get<Component::Draggable>(entity).radius;
        float top_left_x = (window.w / 2.0f) - radius;
        float top_left_y = (window.h / 2.0f) - radius;
        registry.replace<Component::Position>(entity, top_left_x, top_left_y - (knob.index * radius));
    }

    void position_knob(entt::registry& registry, entt::entity entity)
    {
        const auto& window = registry.ctx<Component::Window>();
        position_knob_w(registry, entity, window);
    }

    void on_window_update(entt::registry& registry, entt::entity entity)
    {
        const auto& window = registry.ctx<Component::Window>();
        auto knobs = registry.view<Component::Knob>();
        for (auto knob : knobs) position_knob_w(registry, knob, window);
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
                knob.value = p[knob.index];
                registry.replace<Component::Knob>(knob_entity, knob);
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
                knob.value = p[knob.index];
                registry.replace<Component::Knob>(knob_entity, knob);
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
            registry.replace<Component::Knob>(entity, knob.index, Simple::clip(knob.value + delta));
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

    Component::Position get_indicator_position(entt::registry& registry, entt::entity knob)
    {
        constexpr float deg_to_rad = Simple::pi / 180.0f;
        auto radius = registry.get<Component::Draggable>(knob).radius/2.0f - 15;
        auto center = registry.get<Component::Position>(knob);
        auto angle = (-120.0f - registry.get<Component::Knob>(knob).value * 300.0f) * deg_to_rad;
        angle = Simple::wrap(angle, -Simple::pi, Simple::pi);
        return {center.x + radius * std::cos(angle), center.y + radius * std::sin(angle)};
    }

    void update_knobview(entt::registry& registry, entt::entity knob)
    {
        auto knobview = registry.get<KnobView>(knob);
        auto ring_color = System::hover_select_color(registry, knob);
        auto fill_color = registry.get<Component::Color>(knob);
        auto position = registry.get<Component::Position>(knob);
        auto radius = registry.get<Component::Draggable>(knob).radius;
        auto indicator_position = get_indicator_position(registry, knob);

        auto emp_or_rep = [&](auto entity, auto radius, auto position, auto color, auto border, auto border_thick)
        {
            registry.emplace_or_replace<Component::Circle>(entity,
                    Component::Circle
                    { {color.r, color.g, color.b, color.a}
                    , {border.r, border.g, border.b, border.a}
                    , {position.x, position.y}
                    , radius
                    , border_thick
                    });
        };

        emp_or_rep(knobview.background, radius, position, ring_color, fill_color, 5.0f);
        emp_or_rep(knobview.indicator, 15.0f, indicator_position, fill_color, Component::Color{0,0,0,1}, 3.0f);
    }

    void construct_knobview(entt::registry& registry, entt::entity knob)
    {
        registry.emplace<KnobView>(knob, registry.create(), registry.create());
        update_knobview(registry, knob);
    }

    void destroy_knobview(entt::registry& registry, entt::entity knob)
    {
        auto& knobview = registry.get<KnobView>(knob);
        registry.destroy(knobview.background);
        registry.destroy(knobview.indicator);
    }
}

namespace System
{
    void Knob::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_construct<Component::Knob>().connect<&position_knob>();
        registry.on_construct<Component::Knob>().connect<&construct_knobview>();
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
            registry.emplace<Component::Position>(knob, 0.0f, 100.0f * i);
            registry.emplace<Component::Selectable>(knob, false, Component::Selectable::Group::Knob);
            registry.emplace<Component::Color>(knob);
            registry.emplace<Component::SelectionHovered>(knob, false);
            registry.emplace<Component::Draggable>(knob, 75.0f);
            registry.emplace<Component::Knob>(knob, i);
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
