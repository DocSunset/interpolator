#include "selection.h"

#include <limits>
#include "components/mouse_button.h"
#include "components/modifier_keys.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/selected.h"

namespace System
{
    struct Selection::Implementation
    {
        Implementation(entt::registry& registry)
        {
            registry.on_update<Component::LeftMouseButton>().connect<&Selection::Implementation::on_mouse_button>(*this);
        }

        void on_mouse_button(entt::registry& registry, entt::registry::entity_type entity)
        {
            static constexpr float threshold = 50;
            using Component::LeftMouseButton;
            using Component::Demo;
            using Component::Position;
            using Component::Selected;
            auto btn = registry.get<LeftMouseButton>(entity);
            if (not btn.pressed) return;
            // find the nearest demo
            entt::entity nearest_entity = entt::null;
            float min_dist = std::numeric_limits<float>::max();
            auto demos = registry.view<Demo, Position>();
            for (auto &&[entity, demo, position] : demos.each())
            {
                auto dist = distance(btn.position, position);
                if (dist < threshold && dist < min_dist)
                {
                    nearest_entity = entity;
                    min_dist = dist;
                }
            }
            if (nearest_entity == entt::null) // click too far to select anything
            {
                // unselect everything
                for (auto &&[entity, selection] : registry.view<Selected>().each())
                {
                    if (selection) registry.replace<Selected>(entity, false);
                }
                return;
            }

            auto selected = registry.get<Selected>(nearest_entity);
            if (selected)
            {
                registry.replace<Selected>(nearest_entity, false);
            }
            else // not already selected
            {
                if (registry.ctx<Component::ShiftModifier>()) // add to selection
                {
                    registry.replace<Selected>(nearest_entity, true);
                    // clear other selection groups
                }
                else // select uniquely
                {
                    for (auto &&[entity, selection] : registry.view<Selected>().each())
                    {
                        if (entity == nearest_entity) // we know that nearest is not already selected
                            registry.replace<Selected>(entity, true);
                        else if (selection) // if already selected, unselect
                            registry.replace<Selected>(entity, false);
                    }
                }
            }
        }

        void run(entt::registry& registry)
        {
        }
    };

    Selection::Selection(entt::registry& registry)
    {
        pimpl = new Implementation(registry);
    }

    Selection::~Selection() { delete pimpl; }

    void Selection::run(entt::registry& registry) { pimpl->run(registry); }
}
