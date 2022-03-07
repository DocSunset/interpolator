#include "manual_color_knobs.h"
#include "components/knob.h"
#include "components/color.h"
#include "components/demo.h"
#include "components/draggable.h"
#include "components/manual_vis.h"
#include <simple/boundaries.h>

namespace
{
    struct ColorKnob {};

    void on_update(entt::registry& registry, entt::entity entity)
    {
        if (not registry.all_of<ColorKnob>(entity)) return;
        auto& knob = registry.get<Component::Knob>(entity);
        std::size_t num_selected_demos = 0;
        auto view = registry.view<Component::Selected, Component::Demo>();
        for (auto entity : view)
        {
            registry.patch<Component::Color>(entity, [&](auto& c)
            {
                c[knob.index] = Simple::clip(c[knob.index] + knob.delta);
            });
            ++num_selected_demos;
        }
        knob.delta = 0;
    }

    void sync_knob_values(entt::registry& registry)
    {
        int n_demos = 0;
        Component::Color color{0.0f,0.0f,0.0f,0.0f};
        auto selected_demos = registry.view<Component::Demo, Component::Selected>();
        for (auto demo : selected_demos)
        {
            ++n_demos;
            auto dcolor = registry.get<Component::Color>(demo);
            color += (dcolor - color) / n_demos;
        }

        if (n_demos == 0) return;

        auto knobs = registry.view<Component::Knob, ColorKnob>();
        for (auto knob : knobs)
        {
            registry.patch<Component::Knob>(knob, [&](auto& knob) {knob.value = color[knob.index];});
        }
    }
}

namespace System
{
    void ManualColorKnobs::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_update<Component::Knob>().connect<&on_update>();
    }

    void ManualColorKnobs::prepare_registry(entt::registry& registry)
    {
        for (int i = 0; i < 3; ++i) // 3 for RGB
        {
            auto knob = registry.create();
            registry.emplace<ColorKnob>(knob);
            registry.emplace<Component::Knob>(knob, i, 0.0f, 65.0f);
            registry.replace<Component::Color>(knob, float(i == 0), float(i == 1), float(i == 2), 1.0f);
        }
    }

    void ManualColorKnobs::prepare_to_paint(entt::registry& registry)
    {
        sync_knob_values(registry);
    }
}
