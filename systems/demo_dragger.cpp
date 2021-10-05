#include "demo_dragger.h"

#include "components/demo.h"
#include "components/draggable.h"
#include "components/position.h"

namespace System
{
    struct DemoDragger::Implementation
    {
        entt::observer dragged;

        Implementation(entt::registry& registry)
            : dragged{registry, entt::collector
                    .update<Component::Draggable>()
                    .where<Component::Demo>()}
        {
        }

        void run(entt::registry& registry)
        {
            dragged.each([&](const auto entity)
            {
                auto& p    = registry.get<Component::Position>(entity);
                auto& drag = registry.get<Component::Draggable>(entity);
                registry.replace<Component::Position>(entity, p + drag.delta);
                drag.delta = {0, 0};
            });
        }
    };

    DemoDragger::DemoDragger(entt::registry& registry)
    {
        pimpl = new Implementation(registry);
    }

    DemoDragger::~DemoDragger()
    {
        free(pimpl);
    }
    
    void DemoDragger::run(entt::registry& registry)
    {
        pimpl->run(registry);
    }
}
