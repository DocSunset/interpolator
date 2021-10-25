#include "demo_dragger.h"

#include "components/demo.h"
#include "components/draggable.h"
#include "components/position.h"

namespace System
{
    struct DemoDragger::Implementation
    {
        entt::observer dragged;

        Implementation()
        {
        }

        void setup_reactive_systems(entt::registry& registry)
        {
            dragged.connect(registry, entt::collector
                    .update<Component::Draggable>()
                    .where<Component::Demo>()
                    );
        }

        void prepare_registry(entt::registry& registry)
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

    DemoDragger::DemoDragger()
    {
        pimpl = new Implementation();
    }

    void DemoDragger::setup_reactive_systems(entt::registry& registry)
    {
        pimpl->setup_reactive_systems(registry);
    }

    void DemoDragger::prepare_registry(entt::registry& registry)
    {
        pimpl->prepare_registry(registry);
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
