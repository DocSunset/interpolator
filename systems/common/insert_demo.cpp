#include "insert_demo.h"
#include "components/draggable.h"
#include "components/paint_flag.h"

namespace System
{

    entt::entity insert_demo(entt::registry& registry
            , const Component::Demo::Source& source
            , const Component::Demo::Destination& destination
            )
    {
        auto demo = registry.create();
        registry.emplace<Component::Demo>(demo, demo);
        registry.replace<Component::Demo::Source>(demo, source);
        registry.replace<Component::Demo::Destination>(demo, destination);
        registry.replace<Component::Selectable>(demo, true, Component::Selectable::Group::Demo);
        registry.emplace<Component::Selected>(demo);
        registry.ctx<Component::PaintFlag>().set();
        return demo;
    }
}
