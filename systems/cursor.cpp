#include "cursor.h"
#include "components/fmsynth.h"
#include "components/position.h"
#include "components/color.h"
#include "components/draggable.h"
#include "components/line.h"
#include "components/paint_flag.h"
#include "components/button.h"
#include "components/demo.h"
#include "systems/common/draggable.h"
#include "systems/common/interpolator.h"
#include "systems/interpolator.h"

namespace
{
    struct CursorView
    {
        entt::entity h;
        entt::entity v;
        static constexpr int radius = 30;
    };

    void update_cursor(entt::registry& registry, entt::entity entity)
    {
        using namespace Component;
        auto viewer = registry.get<CursorView>(entity);
        auto position = registry.get<Position>(entity);
        auto color = System::hover_select_color(registry, entity
                , Color{0.5,0.5,0.5,1.0}
                , Color{0.7,0.8,0.8,1.0}
                , Color{1.0,0.7,0.7,1.0}
                );

        registry.replace<FMSynthParameters>(entity, System::query(registry, position));

        Line line_h, line_v;
        line_h = line_v = Line
            { .color = {color.r, color.g, color.b, color.a}
            , .border = {color.r, color.g, color.b, color.a}
            , .start_position = {position.x, position.y}
            , .end_position = {position.x, position.y}
            , .line_thickness = 1
            , .border_thickness = 0
            , .border_transition = 0
            , .blur_radius = 1
            , .cap = Line::Cap::None
            };

        line_h.start_position[0] = line_h.start_position[0] - CursorView::radius;
        line_h.end_position[0] = line_h.end_position[0] + CursorView::radius;
        line_v.start_position[1] = line_v.start_position[1] - CursorView::radius;
        line_v.end_position[1] = line_v.end_position[1] + CursorView::radius;

        registry.emplace_or_replace<Line>(viewer.h, line_h);
        registry.emplace_or_replace<Line>(viewer.v, line_v);
        registry.ctx<PaintFlag>().set();
    }

    struct NewDemoButton {};

    void maybe_make_new_demo(entt::registry& registry, entt::entity entity)
    {
        if (not registry.all_of<NewDemoButton>(entity)) return;
        for (auto cursor : registry.view<CursorView>())
        {
            auto demo = registry.create();
            auto position = registry.get<Component::Position>(cursor);
            auto fm = registry.get<Component::FMSynthParameters>(cursor);
            registry.emplace<Component::Demo>(demo, (long long)demo);
            registry.replace<Component::Position>(demo, position);
            registry.replace<Component::FMSynthParameters>(demo, fm);
        }
    }
}

namespace System
{
    void Cursor::setup_reactive_systems(entt::registry& registry)
    {
        dragged.connect(registry, entt::collector
                .update<Component::Draggable>()
                .where<CursorView>()
                );
        updated.connect(registry, entt::collector
                .update<Component::Position>().where<CursorView>()
                .update<Component::Selectable>().where<CursorView>()
                .update<Component::SelectionHovered>().where<CursorView>()
                );
        registry.on_construct<Component::ButtonPress>().connect<&maybe_make_new_demo>();
    }

    void Cursor::prepare_registry(entt::registry& registry)
    {
        auto cursor = registry.create();
        registry.emplace<Component::FMSynthParameters>(cursor);
        registry.emplace<Component::Position>(cursor, 0.0f, 0.0f);
        registry.emplace<Component::Selectable>(cursor, false, Component::Selectable::Group::Cursor);
        registry.emplace<Component::SelectionHovered>(cursor, false);
        registry.emplace<Component::Draggable>(cursor
                , 25.0f
                , Component::Position::Zero()
                , Component::Position::Zero()
                , Component::Position::Zero()
                , Component::Position::Zero()
                );
        auto viewer = CursorView{registry.create(), registry.create()};
        registry.emplace<CursorView>(cursor, viewer);
        update_cursor(registry, cursor);

        auto btn = registry.create();
        registry.emplace<NewDemoButton>(btn);
        registry.emplace<Component::Button>(btn, 50.0f);
        registry.emplace<Component::Position>(btn, -200.0f, -200.0f);
        registry.emplace<Component::Color>(btn, 0.5f, 0.9f, 0.7f, 1.0f);
    }

    void Cursor::run(entt::registry& registry)
    {
        drag_update_position(registry, dragged);
        updated.each([&](auto entity){update_cursor(registry, entity);});
    }
}
