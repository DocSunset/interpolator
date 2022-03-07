#include "demo_dest_knobs.h"
#include "components/knob.h"
#include "components/color.h"
#include "components/draggable.h"
#include "components/demo.h"
#include "components/position.h"
#include "systems/common/vis.h"
#include <simple/boundaries.h>

namespace
{
    struct DemoKnob {};

    void on_update(entt::registry& registry, entt::entity entity)
    {
        if (not registry.all_of<DemoKnob>(entity)) return;
        const auto& knob = registry.get<Component::Knob>(entity);
        std::size_t num_selected_demos = 0;
        auto view = registry.view<Component::Selected, Component::Demo>();
        for (auto entity : view)
        {
            registry.patch<Component::Demo::Destination>(entity, [&](auto& p)
            {
                p[knob.index] = Simple::clip(p[knob.index] + knob.delta);
            });
            ++num_selected_demos;
        }
        if (num_selected_demos == 0)
        {
            auto& p = registry.ctx<Component::Demo::Destination>();
            p[knob.index] = knob.value;
        }
    }

    void sync_knob_values(entt::registry& registry)
    {
        registry.on_update<Component::Knob>().disconnect<&on_update>();
        auto knobs = registry.view<Component::Knob, DemoKnob>();
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
                auto dest = registry.get<Component::Demo::Destination>(demo);
                p += dest / n_demos;
                color += dcolor / n_demos;
            }
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
        registry.on_update<Component::Knob>().connect<&on_update>();
    }
}

namespace System
{
    void DemoDestKnobs::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_update<Component::Knob>().connect<&on_update>();
    }

    void DemoDestKnobs::prepare_registry(entt::registry& registry)
    {
        for (int i = 0; i < Component::Demo::num_destinations; ++i)
        {
            auto knob = registry.create();
            registry.emplace<DemoKnob>(knob);
            registry.emplace<Component::Knob>(knob, i, 0.0f, 75.0f);
        }
    }

    void DemoDestKnobs::prepare_to_paint(entt::registry& registry)
    {
        sync_knob_values(registry);
    }
}
