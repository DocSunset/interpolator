#include "demo_maker.h"
#include "components/demo.h"
#include "components/color.h"
#include "components/position.h"
#include "components/draggable.h"

namespace
{
    void prepare_demo(entt::registry& registry, entt::entity entity)
    {
        registry.emplace<Component::Demo::Source>(entity
                , Component::Demo::Source::Random().array() * 0.5f + 0.5f
                );
        registry.emplace<Component::Demo::Destination>(entity
                , Component::Demo::Destination::Random().array() * 0.5f + 0.5f
                );
        registry.emplace<Component::Selectable>(entity, false, Component::Selectable::Group::Demo);
        registry.emplace<Component::SelectionHovered>(entity, false);
        registry.emplace<Component::Draggable>(entity, 25.0f);
    }
}

namespace System
{
    void DemoMaker::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_construct<Component::Demo>().connect<&prepare_demo>();
    }

    void DemoMaker::prepare_registry(entt::registry& registry)
    {
        for (int i = 0; i < 5; ++i)
        {
            auto entity = registry.create();
            registry.emplace<Component::Demo>(entity, entity);
        }
    }

    DemoMaker::~DemoMaker() {}
}
