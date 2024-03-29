#include "editor.h"
#include "components/cursor.h"
#include "components/draggable.h"
#include "components/flag.h"
#include "components/button.h"
#include "components/paint_flag.h"
#include "components/demo.h"
#include "components/window.h"
#include "components/vis.h"
#include "components/color.h"
#include "components/position.h"
#include "components/manual_vis.h"
#include "components/smooth_position.h"

#include "common/vis.h"
#include "common/interpolator.h"
#include "common/draggable.h"
#include "common/editor.h"

namespace
{
    struct EditCursor{entt::entity entity;};
    struct InteractCursor{entt::entity entity;};
    struct ManualVisFlag{entt::entity entity;};
    struct NewDemoButton {};
    struct DeleteDemoButton {};
    struct VisModeButton {};

    constexpr unsigned int cursor_radius = 30;
    constexpr float button_radius = 50;

    void update_window(entt::registry& registry, entt::entity entity)
    {
        const auto& window = registry.ctx<Component::Window>();
        constexpr float padding = 10;
        constexpr float step = padding + button_radius;
        const float start[] = { -window.w/2.0f + step
                              , -window.h/2.0f + step
                              };

        int i = 0;
        auto position_button = [&](auto button_entity)
        {
            registry.replace<Component::Position>(button_entity, start[0] + i++ * step, start[1]);
        };

        for (auto ent : registry.view<NewDemoButton>()) position_button(ent);
        for (auto ent : registry.view<DeleteDemoButton>()) position_button(ent);
        for (auto ent : registry.view<VisModeButton>()) position_button(ent);
    }

    void demo_buttons(entt::registry& registry, entt::entity entity)
    {
        if (registry.all_of<NewDemoButton>(entity))
        {
            const auto cursor_entity = registry.ctx<EditCursor>().entity;
            const auto& position = registry.get<Component::Position>(cursor_entity);
            const auto source = System::position_to_source(registry, position);
            const auto destination = registry.ctx<Component::Demo::Destination>();
            auto color = System::destination_to_color(registry, destination);
            System::insert_demo(registry, source, destination, position, color);
        }
        else if (registry.all_of<DeleteDemoButton>(entity))
        {
            System::delete_selected_demos(registry);
        }
        else if (registry.all_of<VisModeButton>(entity))
        {
            auto& mv = registry.ctx<Component::ManualVis>();
            mv.toggle();
            registry.replace<Component::ManualVis>(registry.ctx<ManualVisFlag>().entity, bool(mv));
        }
    }
}

namespace System
{
    void Editor::setup_reactive_systems(entt::registry& registry)
    {
        dragged.connect(registry, entt::collector
                .update<Component::Draggable>()
                .where<EditCursor>()
                );

        registry.on_construct<Component::ButtonPress>().connect<&demo_buttons>();
    }

    void Editor::prepare_registry(entt::registry& registry)
    {
        auto manual_vis_entity = registry.create();
        registry.set<ManualVisFlag>(manual_vis_entity);
        registry.emplace<Component::ManualVis>(manual_vis_entity, registry.set<Component::ManualVis>(false));

        auto edit_cursor_entity = registry.create();
        registry.emplace<EditCursor>(edit_cursor_entity, edit_cursor_entity);
        registry.emplace<Component::Cursor>(edit_cursor_entity, cursor_radius);
        registry.emplace<Component::Draggable>(edit_cursor_entity
                , (float)cursor_radius
                , Component::Position::Zero()
                , Component::Position::Zero()
                , Component::Position::Zero()
                , Component::Position::Zero()
                );
        registry.emplace<Component::Demo::Source>(edit_cursor_entity);
        registry.set<EditCursor>(EditCursor{edit_cursor_entity});

        auto interact_cursor_entity = registry.create();
        registry.emplace<InteractCursor>(interact_cursor_entity, interact_cursor_entity);
        registry.emplace<Component::Cursor>(interact_cursor_entity, cursor_radius);
        registry.emplace<Component::Demo::Source>(interact_cursor_entity);
        registry.emplace<Component::Demo::Destination>(interact_cursor_entity);
        registry.emplace<Component::SmoothPosition>(interact_cursor_entity);
        registry.emplace<Component::Vis>(interact_cursor_entity);
        registry.set<InteractCursor>(InteractCursor{interact_cursor_entity});

        auto new_demo_button = registry.create();
        registry.emplace<NewDemoButton>(new_demo_button);
        registry.emplace<Component::Button>(new_demo_button, button_radius);
        registry.emplace<Component::Position>(new_demo_button, -200.0f, -200.0f);
        registry.emplace<Component::Color>(new_demo_button, 0.2f, 0.7f, 0.2f, 1.0f);

        auto delete_demo_button = registry.create();
        registry.emplace<DeleteDemoButton>(delete_demo_button);
        registry.emplace<Component::Button>(delete_demo_button, button_radius);
        registry.emplace<Component::Position>(delete_demo_button, -145.0f, -200.0f);
        registry.emplace<Component::Color>(delete_demo_button, 0.7f, 0.2f, 0.2f, 1.0f);

        auto vis_mode_button = registry.create();
        registry.emplace<VisModeButton>(vis_mode_button);
        registry.emplace<Component::Button>(vis_mode_button, button_radius);
        registry.emplace<Component::Position>(vis_mode_button, -80.0f, -200.0f);
        registry.emplace<Component::Color>(vis_mode_button, 0.99f, 0.99f, 0.0f, 1.0f);

        registry.on_update<Component::Window>().connect<&update_window>();

        registry.set<Component::CursorMode>(Component::CursorMode::Edit);
    }

    void Editor::run(entt::registry& registry)
    {
        if (dragged.size())
        {
            drag_update_position(registry, dragged);
        }
    }

    void Editor::prepare_to_paint(entt::registry& registry)
    {
        constexpr auto hide = [](auto& cursor) {cursor.hidden = true;};
        constexpr auto show = [](auto& cursor) {cursor.hidden = false;};
        auto interact_cursor = registry.ctx<InteractCursor>().entity;
        auto edit_cursor = registry.ctx<EditCursor>().entity;
        auto source = registry.ctx<Component::Demo::Source>();
        auto destination = source_to_destination(registry, source);
        registry.replace<Component::Demo::Source>(interact_cursor, source);
        registry.replace<Component::Demo::Destination>(interact_cursor, destination);

        if (registry.ctx<Component::ManualVis>())
        {
            registry.replace<Component::Position>(interact_cursor, source_to_position(registry, source));
            registry.replace<Component::Color>(interact_cursor, destination_to_color(registry, destination));
        }
        if (destination != registry.ctx<Component::Demo::Destination>())
        {
            registry.patch<Component::Cursor>(interact_cursor, hide);
            registry.patch<Component::Cursor>(edit_cursor, show);
            registry.set<Component::CursorMode>(Component::CursorMode::Edit);
        }
        else
        {
            registry.patch<Component::Cursor>(interact_cursor, show);
            registry.patch<Component::Cursor>(edit_cursor, hide);
            registry.set<Component::CursorMode>(Component::CursorMode::Interact);
        }
    }
}
