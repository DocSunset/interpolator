#include "demonstration.h"
#include "components/demo.h"
#include "components/draggable.h"
#include "components/color.h"
#include "components/circle.h"
#include "components/vis.h"
#include "components/libmapper_editor.h"
#include "components/cursor.h"
#include "systems/common/draggable.h"

namespace
{
    void update_circle(entt::registry& registry, entt::entity entity)
    {
        auto mapper_hovered = registry.all_of<Component::LibmapperHovered>(entity);
        auto ring_color = mapper_hovered && registry.ctx<Component::CursorMode>() == Component::CursorMode::Interact
                ?  Component::Color{0.9f,0.9f,0.1f,1.0f} 
                : System::hover_select_color(registry, entity);
        auto fill_color = registry.get<Component::Color>(entity);
        auto position = registry.get<Component::Position>(entity);

        registry.emplace_or_replace<Component::Circle>(entity,
                    Component::Circle
                    { {fill_color[0], fill_color[1], fill_color[2], fill_color[3]}
                    , {ring_color[0], ring_color[1], ring_color[2], ring_color[3]}
                    , {position.x, position.y}
                    , Component::Demo::radius
                    , 5
                    });
    }

    // before a demonstration is constructed, the user may first emplace source,
    // destination, position, and color attributes. If these are not already attached
    // to the entity when the demonstration is constructed, defaults will be added.
    void prepare_demo(entt::registry& registry, entt::entity entity)
    {
        registry.emplace<Component::Draggable>(entity, Component::Demo::radius);
        registry.emplace<Component::Vis>(entity);
        update_circle(registry, entity);
    }
}

namespace System
{
    void Demonstration::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_construct<Component::Demo>().connect<&prepare_demo>();
        dragged.connect(registry, entt::collector
                .update<Component::Draggable>()
                .where<Component::Demo>()
                );
        updated_demos.connect(registry, entt::collector
                .update<Component::Position>().where<Component::Demo>()
                .update<Component::Color>().where<Component::Demo>()
                .update<Component::Selectable>().where<Component::Demo>()
                .update<Component::SelectionHovered>().where<Component::Demo>()
                .group<Component::LibmapperHovered>().where<Component::Demo>()
                .group<Component::Demo>(entt::exclude<Component::LibmapperHovered>)
                );
    }

    void Demonstration::run(entt::registry& registry)
    {
        drag_update_position(registry, dragged);
    }

    void Demonstration::prepare_to_paint(entt::registry& registry)
    {
        updated_demos.each([&](auto entity){update_circle(registry, entity);});
    }

    Demonstration::~Demonstration() {}
}
