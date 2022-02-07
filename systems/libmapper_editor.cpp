#include "libmapper_editor.h"
#include <limits>
#include "components/libmapper_editor.h"
#include "components/draggable.h"
#include "components/paint_flag.h"
#include "common/insert_demo.h"
#include "common/delete_demo.h"

namespace
{
    auto get_nearest(entt::registry& registry, const Component::Demo::Source& p1)
    {
        entt::entity nearest = entt::null;
        float distance = std::numeric_limits<float>::max();
        for (auto entity : registry.view<Component::Demo>())
        {
            auto p2 = registry.get<Component::Demo::Source>(entity);
            float d = (p1 - p2).norm();
            if (d < distance)
            {
                distance = d;
                nearest = entity;
            }
        }
        return nearest;
    }

    struct Grabbed {entt::entity entity;};

    void hover(entt::registry& registry, entt::entity nearest)
    {
        if (nearest == entt::null) return;
        if (registry.all_of<Component::LibmapperHovered>(nearest)) return;
        for (auto entity : registry.view<Component::LibmapperHovered>())
            registry.erase<Component::LibmapperHovered>(entity);
        registry.emplace<Component::LibmapperHovered>(nearest);
        registry.ctx<Component::PaintFlag>().set();
    }

    void grab(entt::registry& registry, entt::entity nearest)
    {
        if (nearest == entt::null) return;
        registry.set<Grabbed>(nearest);
    }

    void drag(entt::registry& registry, const Component::Demo::Source& position)
    {
        auto grabbed = registry.ctx<Grabbed>().entity;
        if (grabbed == entt::null) return;
        if (registry.all_of<Component::Grabbed>(grabbed)) return; // don't fight with draggable system
        registry.replace<Component::Demo::Source>(grabbed, position);
        registry.ctx<Component::PaintFlag>().set();
    }

    void drop(entt::registry& registry)
    {
        registry.set<Grabbed>(entt::null);
    }

    void on_grab(entt::registry& registry, entt::entity entity)
    {
        const auto& grab_comp = registry.get<Component::Grab>(entity);
        switch (grab_comp.state)
        {
            case Component::Grab::State::Hovering:
                hover(registry, get_nearest(registry, grab_comp.position));
                break;
            case Component::Grab::State::Grabbing:
                grab(registry, get_nearest(registry, grab_comp.position));
                break;
            case Component::Grab::State::Dragging:
                drag(registry, grab_comp.position);
                break;
            case Component::Grab::State::Dropping:
                drop(registry);
                break;
        }
    }

    void delete_demo(entt::registry& registry, entt::entity _)
    {
        const auto& source = registry.ctx<Component::Demo::Source>();
        auto nearest = get_nearest(registry, source);
        if (nearest == entt::null) return;
        System::delete_demo(registry, get_nearest(registry, source));
    }

    void insert_demo(entt::registry& registry, entt::entity _)
    {
        const auto& source = registry.ctx<Component::Demo::Source>();
        const auto& destination = registry.ctx<Component::Demo::Destination>();
        System::insert_demo(registry, source, destination);
    }
}

namespace System
{
    void LibmapperEditor::prepare_registry(entt::registry& registry)
    {
        registry.set<Grabbed>(entt::null);
    }

    void LibmapperEditor::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_update<Component::Grab>().connect<&on_grab>();
        registry.on_update<Component::LibmapperDeleteDemo>().connect<&::delete_demo>();
        registry.on_update<Component::LibmapperInsertDemo>().connect<&::insert_demo>();
    }
}
