#include "manual_color_knobs.h"
#include "components/knob.h"
#include "components/color.h"
#include "components/demo.h"
#include "components/draggable.h"
#include "components/manual_vis.h"
#include "components/window.h"
#include <simple/boundaries.h>

namespace
{
    constexpr float radius = 65;

    struct ColorKnob {};

    void on_update(entt::registry& registry, entt::entity entity)
    {
        if (not registry.all_of<ColorKnob>(entity)) return;
        auto& knob = registry.get<Component::Knob>(entity);
        if (knob.delta == 0) return;

        // drag knob
        std::size_t num_selected_demos = 0;
        auto view = registry.view<Component::Selected, Component::Demo>();
        for (auto entity : view)
        {
            registry.patch<Component::ManualColor>(entity, [&](auto& c)
            {
                c.value[knob.index] = Simple::clip(c.value[knob.index] + knob.delta);
            });
            ++num_selected_demos;
        }
        knob.delta = 0;
    }

    void position_knobs(entt::registry& registry, entt::entity selected_demo_entity)
    {
        using Component::Position;
        auto pos = registry.get<Position>(selected_demo_entity);
        auto win = registry.ctx<Component::Window>();
        auto pad = radius + 5;
        Position start = {pos.x + pad, pos.y};
        if (start.x > win.w/2.0f - pad) start.x = pos.x - (radius + 5);
        if (start.y < -win.h/2.0f + 3 * pad) start.y = -win.h/2.0f + 3*pad;
        else if (start.y > win.h/2.0f - pad) start.y = win.h/2 - pad;
        int i = 0;
        for (auto entity : registry.view<ColorKnob>())
            registry.replace<Position>(entity, Position{start.x, start.y - i++ * pad});
    }

    void sync_knob_values(entt::registry& registry)
    {
        int n_demos = 0;
        Component::Color color{0.0f,0.0f,0.0f,0.0f};
        auto selected_demos = registry.view<Component::Demo, Component::Selected>();
        for (auto demo : selected_demos)
        {
            ++n_demos;
            auto dcolor = registry.get<Component::ManualColor>(demo).value;
            color += (dcolor - color) / n_demos;
        }

        if (n_demos == 0) return;

        auto knobs = registry.view<Component::Knob, ColorKnob>();
        for (auto knob : knobs)
        {
            registry.patch<Component::Knob>(knob, [&](auto& knob) {knob.value = color[knob.index];});
        }
    }

    void make_knobs(entt::registry& registry)
    {
        for (int i = 0; i < 3; ++i) // 3 for RGB
        {
            auto knob = registry.create();
            registry.emplace<ColorKnob>(knob);
            registry.emplace<Component::Knob>(knob, i, 0.0f, radius);
            registry.replace<Component::Color>(knob, float(i == 0), float(i == 1), float(i == 2), 1.0f);
        }
    }

    void destroy_knobs(entt::registry& registry)
    {
        for (auto entity : registry.view<ColorKnob>()) registry.destroy(entity);
    }

    void maybe_destroy_knobs(entt::registry& registry)
    {
        auto view = registry.view<Component::Selected, Component::Demo>();
        for (auto _ : view) return; // return if there are any selected demos
        destroy_knobs(registry);
    }

    void update_knobs(entt::registry& registry)
    {
        auto view = registry.view<Component::Selected, Component::Demo>();
        for (auto entity : view)
        {
            if (registry.view<ColorKnob>().size() == 0) make_knobs(registry);
            position_knobs(registry, entity);
            sync_knob_values(registry);
            return;
        }
    }

    void vis_mode(entt::registry& registry, entt::entity _)
    {
        auto manual = registry.ctx<Component::ManualVis>();
        if (manual)
        {
            registry.on_update<Component::Knob>().connect<&on_update>();
            registry.on_construct<Component::Selected>().connect<&update_knobs>();
            update_knobs(registry);
        }
        else
        {
            registry.on_update<Component::Knob>().disconnect<&on_update>();
            registry.on_construct<Component::Selected>().disconnect<&update_knobs>();
            destroy_knobs(registry);
        }
    }
}

namespace System
{
    void ManualColorKnobs::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_update<Component::ManualVis>().connect<&vis_mode>();
        dragged_demos.connect(registry, entt::collector
                .update<Component::Position>()
                .where<Component::Demo>()
                .where<Component::Selected>()
                );
    }

    void ManualColorKnobs::run(entt::registry& registry)
    {
        if (registry.ctx<Component::ManualVis>())
        {
            maybe_destroy_knobs(registry);
            if (not dragged_demos.empty()) 
            {
                update_knobs(registry);
                dragged_demos.clear();
            }
        }
        else dragged_demos.clear();
    }

    void ManualColorKnobs::prepare_to_paint(entt::registry& registry)
    {
        sync_knob_values(registry);
    }
}
