#include "source_knobs.h"
#include "components/knob.h"
#include "components/color.h"
#include "components/draggable.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/window.h"
#include "systems/common/vis.h"
#include <simple/boundaries.h>

namespace
{
    struct SourceKnob {};

    void position_knob(entt::registry& registry, entt::entity entity, float& left, float& top)
    {
        constexpr float padding = 5;
        const auto& window = registry.ctx<Component::Window>();
        auto knob = registry.get<Component::Knob>(entity);
        float radius = padding + knob.radius;
        top = top - radius;
        if (top - radius < -window.h/2.0f + 100)
        {
            top = window.h/2.0f - radius;
            left = left + padding + radius;
        }

        registry.emplace_or_replace<Component::Position>(entity, left + radius, top);
    }

    void position_knobs(entt::registry& registry, entt::entity _)
    {
        auto knobs = registry.view<SourceKnob, Component::Knob>();
        const auto& window = registry.ctx<Component::Window>();
        float left = -window.w / 2.0f;
        float top = window.h / 2.0f;
        int i = 0;
        for (auto knob : knobs) position_knob(registry, knob, left, top);
    }

    void on_update(entt::registry& registry, entt::entity entity)
    {
        if (not registry.all_of<SourceKnob>(entity)) return;
        const auto& knob = registry.get<Component::Knob>(entity);
        std::size_t num_selected_demos = 0;
        auto view = registry.view<Component::Selected, Component::Demo>();
        for (auto entity : view)
        {
            registry.patch<Component::Demo::Source>(entity, [&](auto& p)
            {
                p[knob.index] = Simple::clip(p[knob.index] + knob.delta);
            });
            ++num_selected_demos;
        }
        if (num_selected_demos == 0)
        {
            auto& p = registry.ctx<Component::Demo::Source>();
            p[knob.index] = knob.value;
        }
    }

    void sync_knob_values(entt::registry& registry)
    {
        registry.on_update<Component::Knob>().disconnect<&on_update>();
        auto knobs = registry.view<Component::Knob, SourceKnob>();
        auto selected_demos = registry.view<Component::Demo, Component::Selected>();
        Component::Color color{0.0f,0.0f,0.0f,0.0f};

        int n_demos = 0;
        for (auto demo : selected_demos) ++n_demos;
        if (n_demos == 0)
        {
            auto p = registry.ctx<Component::Demo::Source>();
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
            Component::Demo::Source p = Component::Demo::Source::Zero();
            for (auto demo : selected_demos)
            {
                auto dcolor = registry.get<Component::Color>(demo);
                auto src = registry.get<Component::Demo::Source>(demo);
                p += src / n_demos;
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
    void SourceKnobs::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_update<Component::Knob>().connect<&on_update>();
        registry.on_construct<Component::Knob>().connect<&position_knobs>();
        registry.on_update<Component::Window>().connect<&position_knobs>();
    }

    void SourceKnobs::prepare_registry(entt::registry& registry)
    {
        for (int i = 0; i < Component::Demo::num_sources; ++i)
        {
            auto knob = registry.create();
            registry.emplace<SourceKnob>(knob);
            registry.emplace<Component::Knob>(knob, i, 0.0f, 55.0f);
        }
    }

    void SourceKnobs::prepare_to_paint(entt::registry& registry)
    {
        sync_knob_values(registry);
    }
}
