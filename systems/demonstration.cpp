#include "demonstration.h"
#include "components/demo.h"
#include "components/draggable.h"
#include "components/color.h"
#include "components/circle.h"
#include "components/vis.h"
#include "components/grab.h"
#include "systems/common/draggable.h"

namespace
{
    constexpr float demo_radius = 25;

    void update_circle(entt::registry& registry, entt::entity entity)
    {
        auto mapper_hovered = registry.all_of<Component::LibmapperHovered>(entity);
        auto ring_color = mapper_hovered ? Component::Color{0.9f,0.9f,0.1f,1.0f} 
                                 : System::hover_select_color(registry, entity);
        auto fill_color = registry.get<Component::Color>(entity);
        auto position = registry.get<Component::Position>(entity);
        auto radius = registry.get<Component::Draggable>(entity).radius;

        registry.emplace_or_replace<Component::Circle>(entity,
                    Component::Circle
                    { {fill_color.r, fill_color.g, fill_color.b, fill_color.a}
                    , {ring_color.r, ring_color.g, ring_color.b, ring_color.a}
                    , {position.x, position.y}
                    , radius
                    , 5
                    });
    }

    void prepare_demo(entt::registry& registry, entt::entity entity)
    {
        registry.emplace<Component::Demo::Source>(entity
                , Component::Demo::Source::Random().array() * 0.5f + 0.5f
                );
        registry.emplace<Component::Demo::Destination>(entity
                , Component::Demo::Destination::Random().array() * 0.5f + 0.5f
                );
        registry.emplace<Component::Draggable>(entity, demo_radius);
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

    void Demonstration::prepare_registry(entt::registry& registry)
    {
        for (int i = 0; i < 5; ++i)
        {
            auto entity = registry.create();
            registry.emplace<Component::Demo>(entity, entity);
        }
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