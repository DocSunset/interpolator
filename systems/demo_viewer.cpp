#include "demo_viewer.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/color.h"
#include "components/window.h"
#include "components/draggable.h"
#include "components/circle.h"
#include "components/paint_flag.h"
#include "gl/vertex_array.h"
#include "systems/common/draggable.h"

#include "shader/demo_viewer.h"

namespace
{
    void update_circle(entt::registry& registry, entt::entity demo)
    {
        auto ring_color = System::hover_select_color(registry, demo);
        auto fill_color = registry.get<Component::Color>(demo);
        auto position = registry.get<Component::Position>(demo);
        auto radius = registry.get<Component::Draggable>(demo).radius;

        registry.emplace_or_replace<Component::Circle>(demo,
                    Component::Circle
                    { {fill_color.r, fill_color.g, fill_color.b, fill_color.a}
                    , {ring_color.r, ring_color.g, ring_color.b, ring_color.a}
                    , {position.x, position.y}
                    , radius
                    , 5
                    });
    }
}

namespace System
{
    void DemoViewer::setup_reactive_systems(entt::registry& registry)
    {
        updated_demos.connect(registry, entt::collector
                .update<Component::Position>().where<Component::Demo>()
                .update<Component::Color>().where<Component::Demo>()
                .update<Component::Selectable>().where<Component::Demo>()
                .update<Component::SelectionHovered>().where<Component::Demo>()
                );
        new_demos.connect(registry, entt::collector.group<Component::Demo>());
    }

    void DemoViewer::run(entt::registry& registry)
    {
        if (new_demos.empty() && updated_demos.empty()) return;
        auto f = [&](auto entity){update_circle(registry, entity);};
        new_demos.each(f);
        updated_demos.each(f);
    }

    DemoViewer::~DemoViewer() {}
}
