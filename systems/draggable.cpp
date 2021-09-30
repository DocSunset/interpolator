#include "draggable.h"

#include <limits>
#include <iostream>
#include "components/mouse_button.h"
#include "components/mouse_motion.h"
#include "components/modifier_keys.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/draggable.h"
#include "entt/entity/entity.hpp"

namespace
{
    /* actions
     */
    void highlight(entt::registry& registry, entt::entity entity)
    {
        registry.replace<Component::SelectionHovered>(entity, true);
    }

    void unhighlight(entt::registry& registry, entt::entity entity)
    {
        registry.replace<Component::SelectionHovered>(entity, false);
    }

    void select(entt::registry& registry, entt::entity entity)
    {
        registry.replace<Component::Selected>(entity, true);
    }

    void unselect(entt::registry& registry, entt::entity entity)
    {
        registry.replace<Component::Selected>(entity, false);
    }

    void toggle_selection(entt::registry& registry, entt::entity entity)
    {
        auto current = registry.get<Component::Selected>(entity);
        registry.replace<Component::Selected>(entity, not current);
    }

    void select_all(entt::registry& registry)
    {
        for (auto entity : registry.view<Component::Selected>())
        {
            select(registry, entity);
        }
    }

    void unselect_all(entt::registry& registry)
    {
        for (auto entity : registry.view<Component::Selected>())
        {
            unselect(registry, entity);
        }
    }

    void untouch_all(entt::registry& registry)
    {
        for (auto entity : registry.view<Component::Selected>())
        {
            auto& selected = registry.get<Component::Selected>(entity);
            selected._touched = false;
        }
    }

    void touch(entt::registry& registry, entt::entity entity)
    {
        auto& selected = registry.get<Component::Selected>(entity);
        selected._touched = true;
    }

    /* tests
     */
    // returns entt::null in case cmp is not close enough to grab
    entt::entity close_enough_to_grab(const entt::registry& registry, Component::Position cmp)
    {
        entt::entity nearest_entity = entt::null;
        float min_dist = std::numeric_limits<float>::max();
        auto draggables = registry.view<Component::Position, Component::Draggable>();
        for (auto &&[entity, position, draggable] : draggables.each())
        {
            auto dist = distance(cmp, position);
            if (dist < draggable.radius && dist < min_dist)
            {
                nearest_entity = entity;
                min_dist = dist;
            }
        }
        return nearest_entity;
    }

    bool selected(const entt::registry& registry, entt::entity entity)
    {
        return registry.get<Component::Selected>(entity);
    }

    bool shift(const entt::registry& registry)
    {
        return registry.ctx<Component::ShiftModifier>();
    }

    bool already_touched(const entt::registry& registry, entt::entity entity)
    {
        return registry.get<Component::Selected>(entity)._touched;
    }
}

namespace System
{
    struct Draggable::Implementation
    {
        enum State
        {
            START,
            MAYBE_DRAG,
            DRAG,
            SELECT_DRAG,
        } state = START;

        entt::entity maybe_drop = entt::null;
        entt::entity hovered = entt::null;

        Implementation(entt::registry& registry)
        {
            registry.on_update<Component::MouseMotion>().connect<&Draggable::Implementation::on_mouse_motion>(*this);
            registry.on_update<Component::LeftMouseButton>().connect<&Draggable::Implementation::on_mouse_button>(*this);
        }

        void on_mouse_motion(entt::registry& registry, entt::registry::entity_type entity)
        {
            using Component::MouseMotion;

            auto motion = registry.get<MouseMotion>(entity);

            switch (state)
            {
                case START:
                {
                    if (hovered != entt::null) unhighlight(registry, hovered);
                    auto hover = close_enough_to_grab(registry, motion.position);
                    if (hover != entt::null) // got a hover
                    {
                        highlight(registry, hover);
                        hovered = hover;
                    }
                    else hovered = entt::null;
                    return;
                }
                case MAYBE_DRAG:
                    state = DRAG;
                case DRAG:
                    return;
                case SELECT_DRAG:
                {
                    auto touched = close_enough_to_grab(registry, motion.position);
                    if (touched == entt::null) return;
                    if (already_touched(registry, touched)) return;
                    toggle_selection(registry, touched);
                    touch(registry, touched);
                    return;
                }
            }
        }

        void on_mouse_button(entt::registry& registry, entt::registry::entity_type entity)
        {
            using Component::LeftMouseButton;

            auto btn = registry.get<LeftMouseButton>(entity);

            switch (state)
            {
                case START:
                {
                    if (not btn.pressed) return;

                    auto grab = close_enough_to_grab(registry, btn.down_position);
                    if (grab != entt::null) // got a grab
                    {
                        if (selected(registry, grab))
                        {
                            maybe_drop = grab;
                            state = MAYBE_DRAG;
                            return;
                        }
                        else
                        {
                            if (not shift(registry)) unselect_all(registry);
                            select(registry, grab);
                            state = DRAG;
                            return;
                        }
                    }
                    else // no grab
                    {
                        if (not shift(registry)) unselect_all(registry);
                        untouch_all(registry);
                        state = SELECT_DRAG;
                        return;
                    }
                    return;
                }

                case MAYBE_DRAG:
                    if (btn.pressed) return; // this should never happen
                    if (shift(registry)) unselect(registry, maybe_drop);
                    else unselect_all(registry);
                    state = START;
                    return;

                case DRAG:
                case SELECT_DRAG:
                    if (btn.pressed) return; // this should never happen
                    state = START;
                    return;
            }
        }

        void run(entt::registry& registry)
        {
        }
    };

    Draggable::Draggable(entt::registry& registry)
    {
        pimpl = new Implementation(registry);
    }

    Draggable::~Draggable() { delete pimpl; }

    void Draggable::run(entt::registry& registry) { pimpl->run(registry); }
}
