#include "libmapper_editor.h"
#include <limits>
#include "components/grab.h"

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
        if (registry.all_of<Component::LibmapperHovered>(nearest)) return;
        for (auto entity : registry.view<Component::LibmapperHovered>())
            registry.erase<Component::LibmapperHovered>(entity);
        registry.emplace<Component::LibmapperHovered>(nearest);
    }

    void grab(entt::registry& registry, entt::entity nearest)
    {
        registry.set<Grabbed>(nearest);
    }

    void drag(entt::registry& registry, const Component::Demo::Source& position)
    {
        auto grabbed = registry.ctx<Grabbed>().entity;
        registry.replace<Component::Demo::Source>(grabbed, position);
    }

    void drop(entt::registry& registry, entt::entity nearest)
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
                drop(registry, get_nearest(registry, grab_comp.position));
                break;
        }
    }
}

namespace System
{
    void LibmapperEditor::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_update<Component::Grab>().connect<&on_grab>();
    }
}
