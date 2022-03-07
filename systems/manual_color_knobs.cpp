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
        const auto& knob = registry.get<Component::Knob>(entity);
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
            registry.emplace<Component::Knob>(knob, i, 0.0f, 75.0f);
            registry.replace<Component::Color>(knob, float(i == 0), float(i == 1), float(i == 2), 1.0f);
        }
    }
}
