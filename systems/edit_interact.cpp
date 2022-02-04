#include "edit_interact.h"
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
#include "systems/common/vis.h"
#include "systems/common/interpolator.h"
#include "systems/common/draggable.h"

namespace
{
    struct EditCursor{entt::entity entity;};
    struct InteractCursor{entt::entity entity;};
    struct NewDemoButton {};
    struct DeleteDemoButton {};
    struct EditInteractButton {bool editing;};
    struct Editing : public Component::Flag {};

    void update_window(entt::registry& registry, entt::entity entity)
    {
        // get window
        // reposition buttons
    }

    void demo_buttons(entt::registry& registry, entt::entity entity)
    {
        if (registry.all_of<NewDemoButton>(entity))
        {
            auto demo = registry.create();
            Component::Demo::Source source;
            Component::Demo::Destination destination;
            if (registry.ctx<Editing>())
            {
                auto entity = registry.ctx<EditCursor>().entity;
                const auto& position = registry.get<Component::Position>(entity);
                source = System::position_to_source(registry, position);
                destination = System::query(registry, source);
            }
            else
            {
                source = registry.ctx<Component::Demo::Source>();
                destination = registry.ctx<Component::Demo::Destination>();
            }
            registry.emplace<Component::Demo>(demo, demo);
            registry.replace<Component::Demo::Source>(demo, source);
            registry.replace<Component::Demo::Destination>(demo, destination);
            registry.replace<Component::Selectable>(demo, true, Component::Selectable::Group::Demo);
            registry.emplace<Component::Selected>(demo);
            registry.ctx<Component::PaintFlag>().set();
        }
        else if (registry.all_of<DeleteDemoButton>(entity))
        {
            auto view = registry.view<Component::Demo, Component::Selected>();
            registry.destroy(view.begin(), view.end());
            registry.ctx<Component::PaintFlag>().set();
        }
        else if (registry.all_of<EditInteractButton>(entity))
        {
            registry.ctx<Editing>().toggle();
        }
    }
}

namespace System
{
    void EditInteract::setup_reactive_systems(entt::registry& registry)
    {
        dragged.connect(registry, entt::collector
                .update<Component::Draggable>()
                .where<EditCursor>()
                );
    }

    void EditInteract::prepare_registry(entt::registry& registry)
    {
        unsigned int radius = 30;

        registry.set<Editing>(true);

        auto edit_cursor_entity = registry.create();
        registry.emplace<EditCursor>(edit_cursor_entity, edit_cursor_entity);
        registry.emplace<Component::Cursor>(edit_cursor_entity, radius);
        registry.emplace<Component::Draggable>(edit_cursor_entity
                , (float)radius
                , Component::Position::Zero()
                , Component::Position::Zero()
                , Component::Position::Zero()
                , Component::Position::Zero()
                );
        registry.set<EditCursor>(EditCursor{edit_cursor_entity});

        auto interact_cursor_entity = registry.create();
        registry.emplace<InteractCursor>(interact_cursor_entity, interact_cursor_entity);
        registry.emplace<Component::Cursor>(interact_cursor_entity, radius);
        registry.emplace<Component::Demo::Source>(interact_cursor_entity);
        registry.emplace<Component::Demo::Destination>(interact_cursor_entity);
        registry.emplace<Component::Vis>(interact_cursor_entity);
        registry.set<InteractCursor>(InteractCursor{interact_cursor_entity});

        auto new_demo_button = registry.create();
        registry.emplace<NewDemoButton>(new_demo_button);
        registry.emplace<Component::Button>(new_demo_button, 50.0f);
        registry.emplace<Component::Position>(new_demo_button, -200.0f, -200.0f);
        registry.emplace<Component::Color>(new_demo_button, 0.2f, 0.7f, 0.2f, 1.0f);

        auto delete_demo_button = registry.create();
        registry.emplace<DeleteDemoButton>(delete_demo_button);
        registry.emplace<Component::Button>(delete_demo_button, 50.0f);
        registry.emplace<Component::Position>(delete_demo_button, -145.0f, -200.0f);
        registry.emplace<Component::Color>(delete_demo_button, 0.7f, 0.2f, 0.2f, 1.0f);

        auto mode_button = registry.create();
        registry.emplace<EditInteractButton>(mode_button);
        registry.emplace<Component::Button>(mode_button, 50.0f);
        registry.emplace<Component::Position>(mode_button, -90.0f, -200.0f);
        registry.emplace<Component::Color>(mode_button, 0.2f, 0.2f, 0.2f, 1.0f);

        registry.on_update<Component::Window>().connect<&update_window>();
    }

    void EditInteract::run(entt::registry& registry)
    {
        drag_update_position(registry, dragged);
    }

    void EditInteract::prepare_to_paint(entt::registry& registry)
    {
        auto entity = registry.ctx<InteractCursor>().entity;
        registry.replace<Component::Demo::Source>(entity, registry.ctx<Component::Demo::Source>());
        registry.replace<Component::Demo::Destination>(entity, registry.ctx<Component::Demo::Destination>());
    }
}
