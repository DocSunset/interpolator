#include "knob_viewer.h"
#include "components/knob.h"
#include "components/position.h"
#include "components/color.h"
#include "components/draggable.h"
#include "components/circle.h"
#include <simple/constants/pi.h>
#include <simple/boundaries.h>

#include "shader/knob_viewer.h"
#include <iostream>

namespace
{
    struct KnobView
    {
        entt::entity background;
        entt::entity indicator;
    };

    void construct_knobview(entt::registry& registry, entt::entity knob)
    {
        auto& knobview = registry.get<KnobView>(knob);
        knobview.background = registry.create();
        knobview.indicator = registry.create();
    }

    void destroy_knobview(entt::registry& registry, entt::entity knob)
    {
        auto& knobview = registry.get<KnobView>(knob);
        registry.destroy(knobview.background);
        registry.destroy(knobview.indicator);
    }

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
        using Component::Color;
        constexpr Color selected_ring{1,0.7,0.7,1};
        constexpr Color default_ring{0.6,0.6,0.6,1};
        constexpr Color highlight_ring{0.7,0.8,0.8,1};

        auto knobview = registry.get<KnobView>(knob);

        auto s = registry.get<Component::Selectable>(knob);
        auto h = registry.get<Component::SelectionHovered>(knob);
        Color ring_color = s ? selected_ring : h ? highlight_ring : default_ring;
        auto fill_color = registry.get<Color>(knob);
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
        emp_or_rep(knobview.indicator, 15.0f, indicator_position, fill_color, Color{0,0,0,1}, 3.0f);
    }
}

namespace System
{
    void KnobViewer::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_construct<Component::Knob>().connect<&entt::registry::emplace<KnobView>>();
        registry.on_destroy<Component::Knob>().connect<&destroy_knobview>();
        registry.on_construct<KnobView>().connect<&construct_knobview>();

        new_knobs.connect(registry, entt::collector.group<Component::Knob>());
        updated_knobs.connect(registry, entt::collector
                .update<Component::Position>().where<Component::Knob>()
                .update<Component::Knob>()
                );
    }

    void KnobViewer::prepare_to_paint(entt::registry& registry)
    {
        auto f = [&](auto entity){update_knobview(registry, entity);};
        new_knobs.each(f);
        updated_knobs.each(f);
    }

    KnobViewer::~KnobViewer() {}
}
