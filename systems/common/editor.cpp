#include "editor.h"
#include "components/draggable.h"
#include "components/paint_flag.h"
#include "components/vis.h"
#include "components/manual_vis.h"

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
        registry.emplace<Component::ManualPosition>(demo, position);
        registry.emplace<Component::Color>(demo, color);
        registry.emplace<Component::ManualColor>(demo, color);
        registry.emplace<Component::Demo>(demo, demo);
        registry.replace<Component::Selectable>(demo, true, Component::Selectable::Group::Demo);
        registry.emplace<Component::Selected>(demo);
        registry.ctx<Component::PaintFlag>().set();
        return demo;
    }

    void delete_selected_demos(entt::registry& registry)
    {
        auto view = registry.view<Component::Demo, Component::Selected>();
        for (auto entity : view) view.get<Component::Demo>(entity).destroyed = true;
        registry.destroy(view.begin(), view.end());
        registry.ctx<Component::PaintFlag>().set();
    }

    void delete_demo(entt::registry& registry, entt::entity entity)
    {
        registry.get<Component::Demo>(entity).destroyed = true; // this is a nasty hack to enable dataset and save file updates to run reactively without using observers. This should not be necessary, and the only place I'm presently aware of where it is required is to enable the edit cursor position to be updated immediately after inserting a demo
        registry.destroy(entity);
        registry.ctx<Component::PaintFlag>().set();
    }
}
