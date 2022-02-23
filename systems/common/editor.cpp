#include "editor.h"
#include "components/draggable.h"
#include "components/paint_flag.h"
#include "components/vis.h"

namespace System
{
    entt::entity insert_demo(entt::registry& registry
            , const Component::Demo::Source& source
            , const Component::Demo::Destination& destination
            , const Component::Position& position
            , const Component::Color& color
            )
    {
        auto demo = registry.create();
        registry.emplace<Component::Demo::Source>(demo, source);
        registry.emplace<Component::Demo::Destination>(demo, destination);
        registry.emplace<Component::Position>(demo, position);
        registry.emplace<Component::Color>(demo, color);
        registry.emplace<Component::Demo>(demo, demo);
        registry.replace<Component::Selectable>(demo, true, Component::Selectable::Group::Demo);
        registry.emplace<Component::Selected>(demo);
        registry.ctx<Component::PaintFlag>().set();
        return demo;
    }

    void delete_selected_demos(entt::registry& registry)
    {
        auto view = registry.view<Component::Demo, Component::Selected>();
        registry.destroy(view.begin(), view.end());
        registry.ctx<Component::PaintFlag>().set();
    }

    void delete_demo(entt::registry& registry, entt::entity entity)
    {
        registry.destroy(entity);
        registry.ctx<Component::PaintFlag>().set();
    }
}
