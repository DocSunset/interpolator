#include "cursor.h"
#include "components/cursor.h"
#include "components/position.h"
#include "components/color.h"
#include "components/draggable.h"
#include "components/line.h"
#include "components/paint_flag.h"
#include "components/button.h"
#include "components/demo.h"
#include "components/draggable.h"
#include "components/paint_flag.h"
#include "components/circle.h"
#include "systems/common/draggable.h"
#include "systems/common/interpolator.h"
#include "systems/interpolator.h"
#include <iostream>

namespace
{
    struct CursorView
    {
        entt::entity h;
        entt::entity v;
        entt::entity r;
    };

    void update_cursor(entt::registry& registry, entt::entity entity)
    {
        using namespace Component;
        auto cursor = registry.get<Cursor>(entity);
        auto viewer = registry.get<CursorView>(entity);
        auto position = registry.get<Position>(entity);
        auto color = registry.all_of<Selectable>(entity) ?
            System::hover_select_color(registry, entity
                , registry.get<Color>(entity)
                )
            : registry.get<Color>(entity);

        Line line_h, line_v;
        line_h = line_v = Line
            { .color = {1,1,1,0.8}
            , .border = {1,1,1,0.8}
            , .start_position = {position.x, position.y}
            , .end_position = {position.x, position.y}
            , .line_thickness = 1
            , .border_thickness = 0
            , .border_transition = 0
            , .blur_radius = 1
            , .cap = Line::Cap::None
            };

        line_h.start_position[0] = line_h.start_position[0] - cursor.radius;
        line_h.end_position[0] = line_h.end_position[0] + cursor.radius;
        line_v.start_position[1] = line_v.start_position[1] - cursor.radius;
        line_v.end_position[1] = line_v.end_position[1] + cursor.radius;

        auto ring = Circle
            { .color = {color[0], color[1], color[2], 0.1}
            , .border = {color[0], color[1], color[2], color[3]}
            , .position = {position.x, position.y}
            , .radius = 12
            , .border_thickness = 3
            };

        registry.emplace_or_replace<Line>(viewer.h, line_h);
        registry.emplace_or_replace<Line>(viewer.v, line_v);
        registry.emplace_or_replace<Circle>(viewer.r, ring);
        registry.ctx<PaintFlag>().set(); // for when this is called from construct_cursor
    }

    void construct_cursor(entt::registry& registry, entt::entity entity)
    {
        auto viewer = CursorView{registry.create(), registry.create(), registry.create()};
        registry.emplace<CursorView>(entity, viewer);
        registry.emplace<Component::Position>(entity);
        registry.emplace<Component::Color>(entity,0.5f,0.5f,0.5f,1.0f);
        update_cursor(registry, entity);
    }
}

namespace System
{
    void Cursor::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_construct<Component::Cursor>().connect<&construct_cursor>();

        updated.connect(registry, entt::collector
                .update<Component::Position>().where<Component::Cursor>()
                .update<Component::Color>().where<Component::Cursor>()
                .update<Component::Demo::Source>().where<Component::Cursor>()
                .update<Component::Demo::Destination>().where<Component::Cursor>()
                .update<Component::Selectable>().where<Component::Cursor>()
                .update<Component::SelectionHovered>().where<Component::Cursor>()
                );
    }

    void Cursor::prepare_to_paint(entt::registry& registry)
    {
        updated.each([&](auto entity){update_cursor(registry, entity);});
    }
}
