#include "circle_painter.h"
#include "components/circle.h"
#include "components/window.h"
#include "components/line.h"
#include "gl/vertex_array.h"

#include "shader/circle.h"

namespace
{
    void circle_update(entt::registry& registry, entt::entity entity)
    {
        auto c = registry.get<Component::Circle>(entity);
        registry.emplace_or_replace<Component::Line>(entity,
                Component::Line
                { {c.color[0], c.color[1], c.color[2], c.color[3]}
                , {c.border[0], c.border[1], c.border[2], c.border[3]}
                , {c.position[0], c.position[1]}
                , {c.position[0], c.position[1]}
                , c.radius
                , c.border_thickness
                , c.border_transition
                , c.blur_radius
                , Component::Line::Cap::Round
                });
    }
}

namespace System
{
    struct CirclePainter::Implementation
    {
        Implementation()
        {
        }

        void setup_reactive_systems(entt::registry& registry)
        {
            registry.on_construct<Component::Circle>().connect<&circle_update>();
            registry.on_update<Component::Circle>().connect<&circle_update>();
        }

        void prepare_registry(entt::registry& registry)
        {
        }

        void run(entt::registry& registry)
        {
        }
    };

    CirclePainter::CirclePainter()
    {
        pimpl = new Implementation();
    }

    void CirclePainter::setup_reactive_systems(entt::registry& registry)
    {
        pimpl->setup_reactive_systems(registry);
    }

    void CirclePainter::prepare_registry(entt::registry& registry)
    {
        pimpl->prepare_registry(registry);
    }


    CirclePainter::~CirclePainter() { delete pimpl; }

    void CirclePainter::run(entt::registry& registry) { pimpl->run(registry); }
}
