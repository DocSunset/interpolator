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
    }

    void update_edit_cursor(entt::registry& registry)
    {
        if (registry.ctx<Component::ManualVis>()) return;
        auto edit_cursor = registry.ctx<EditCursor>().entity;
        auto source = registry.get<Component::Demo::Source>(edit_cursor);
        auto position = System::source_to_position(registry, source);
        registry.replace<Component::Position>(edit_cursor, position);
    }

    void demo_buttons(entt::registry& registry, entt::entity entity)
    {
        if (registry.all_of<NewDemoButton>(entity))
        {
            const auto cursor_entity = registry.ctx<EditCursor>().entity;
            const auto& position = registry.get<Component::Position>(cursor_entity);
            const auto source = System::position_to_source(registry, position);
            const auto destination = registry.ctx<Component::Demo::Destination>();
            if (registry.ctx<Component::ManualVis>())
            {
                auto color = System::destination_to_color(registry, destination);
                System::insert_demo(registry, source, destination, position, color);
            }
            else
            {
                System::insert_demo(registry, source, destination);
                update_edit_cursor(registry);
            }
        }
        else if (registry.all_of<DeleteDemoButton>(entity))
        {
            System::delete_selected_demos(registry);
            update_edit_cursor(registry);
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
        registry.emplace<ManualVisFlag>(manual_vis_entity, manual_vis_entity);
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

        registry.on_update<Component::Window>().connect<&update_window>();
    }

    void Editor::run(entt::registry& registry)
    {
        if (dragged.size())
        {
            drag_update_position(registry, dragged);
            auto edit_cursor = registry.ctx<EditCursor>().entity;
            auto position = registry.get<Component::Position>(edit_cursor);
            auto source = position_to_source(registry, position);
            registry.replace<Component::Demo::Source>(edit_cursor, source);
        }
    }

    void Editor::prepare_to_paint(entt::registry& registry)
    {
        constexpr auto hide = [](auto& color) {color[3] = 0.2;};
        constexpr auto show = [](auto& color) {color[3] = 1;};
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
            registry.patch<Component::Color>(interact_cursor, hide);
            registry.patch<Component::Color>(edit_cursor, show);
        }
        else
        {
            registry.patch<Component::Color>(interact_cursor, show);
            registry.patch<Component::Color>(edit_cursor, hide);
        }
    }
}
