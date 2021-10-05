#include "demo_maker.h"
#include "components/demo.h"
#include "components/color.h"
#include "components/position.h"
#include "components/draggable.h"

namespace
{
    void prepare_demo(entt::registry& registry, entt::entity demo_entity)
    {
        registry.emplace_or_replace<Component::Color>(demo_entity, Component::Color::Random());
        registry.emplace<Component::Position>(demo_entity, Component::Position::Random());
        registry.emplace<Component::Selected>(demo_entity, false);
        registry.emplace<Component::SelectionHovered>(demo_entity, false);
        registry.emplace<Component::Draggable>(demo_entity
                , 25
                , Component::Position::Zero()
                , Component::Position::Zero()
                , Component::Position::Zero()
                , Component::Position::Zero()
                );
    }
}

namespace System
{
    DemoMaker::DemoMaker()
    {
    }

    void DemoMaker::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_construct<Component::Demo>().connect<&prepare_demo>();
    }

    void DemoMaker::prepare_registry(entt::registry& registry)
    {
        for (int i = 0; i < 5; ++i)
        {
            auto entity = registry.create();
            registry.emplace<Component::Demo>(entity, i);
        }
    }

    void DemoMaker::run(entt::registry& registry)
    {
    }
}
