#include "draggable.h"

#include <limits>
#include <iostream>
#include "components/mouse_button.h"
#include "components/mouse_motion.h"
#include "components/modifier_keys.h"
#include "components/demo.h"
#include "components/knob.h"
#include "components/position.h"
#include "components/draggable.h"
#include "entt/entity/entity.hpp"
#include "components/fmsynth.h"

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
        auto s = registry.get<Component::Selectable>(entity);
        registry.replace<Component::Selectable>(entity, true, s.group);
        registry.emplace_or_replace<Component::Selected>(entity);
    }

    void unselect(entt::registry& registry, entt::entity entity)
    {
        auto s = registry.get<Component::Selectable>(entity);
        registry.replace<Component::Selectable>(entity, false, s.group);
        if (registry.all_of<Component::Selected>(entity))
            registry.erase<Component::Selected>(entity);
    }

    void toggle_selection(entt::registry& registry, entt::entity entity)
    {
        auto current = registry.get<Component::Selectable>(entity);
        if (current) unselect(registry, entity);
        else select(registry, entity);
    }

    void select_all(entt::registry& registry
            , Component::Selectable::Group group = Component::Selectable::Group::All
            )
    {
        for (auto entity : registry.view<Component::Selectable>())
        {
            if (group == Component::Selectable::Group::All
                || registry.get<Component::Selectable>(entity).group == group)
                select(registry, entity);
        }
    }

    void unselect_all(entt::registry& registry
            , Component::Selectable::Group group = Component::Selectable::Group::All
            )
    {
        for (auto entity : registry.view<Component::Selectable>())
        {
            if (group == Component::Selectable::Group::All
                || registry.get<Component::Selectable>(entity).group == group)
                unselect(registry, entity);
        }
    }

    void untouch_all(entt::registry& registry)
    {
        for (auto entity : registry.view<Component::Selectable>())
        {
            auto& selected = registry.get<Component::Selectable>(entity);
            selected._touched = false;
        }
    }

    void touch(entt::registry& registry, entt::entity entity)
    {
        auto& selected = registry.get<Component::Selectable>(entity);
        selected._touched = true;
    }

    void drag(entt::registry& registry, const Component::MouseMotion& motion, Component::Selectable::Group group)
    {
        auto draggables = registry.view<Component::Selected, Component::Selectable, Component::Draggable>();
        for (auto &&[entity, selectable, draggable] : draggables.each())
        {
            if (group != Component::Selectable::Group::All && selectable.group != group) return;
            registry.replace<Component::Draggable>(entity
                    , draggable.radius
                    , draggable.start
                    , draggable.current // previous = current
                    , motion.position // current = motion.position

                    // systems should clear delta to keep track of delta since
                    // last they checked
                    , motion.delta + draggable.delta
                    );
        }
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
        return registry.all_of<Component::Selected>(entity);
    }

    bool shift(const entt::registry& registry)
    {
        return registry.ctx<Component::ShiftModifier>();
    }

    bool already_touched(const entt::registry& registry, entt::entity entity)
    {
        return registry.get<Component::Selectable>(entity)._touched;
    }

    Component::Selectable::Group group(const entt::registry& registry, entt::entity entity)
    {
        return registry.get<Component::Selectable>(entity).group;
    }

    bool any_selected_in_group(const entt::registry& registry, Component::Selectable::Group group)
    {
        switch(group)
        {
            case Component::Selectable::Group::Demo:
                registry.view<Component::Demo, Component::Selected>().each([](auto){return true;});
                break;
            case Component::Selectable::Group::Knob:
                registry.view<Component::Knob, Component::Selected>().each([](auto){return true;});
                break;
            case Component::Selectable::Group::All:
                registry.view<Component::Demo, Component::Selected>().each([](auto){return true;});
                registry.view<Component::Knob, Component::Selected>().each([](auto){return true;});
                break;
        }
        return false;
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
            MAYBE_SELECT_DRAG,
            SELECT_DRAG,
        } state = START;

        entt::entity maybe_drop = entt::null;
        entt::entity hovered = entt::null;
        Component::Selectable::Group last_group = Component::Selectable::Group::All;

        Implementation()
        {
        }

        void setup_reactive_systems(entt::registry& registry)
        {
            registry.on_update<Component::MouseMotion>()
                .connect<&Draggable::Implementation::on_mouse_motion>(*this);
            registry.on_update<Component::LeftMouseButton>()
                .connect<&Draggable::Implementation::on_mouse_button>(*this);
        }

        void prepare_registry(entt::registry& registry)
        {
        }

        void on_mouse_motion(entt::registry& registry, entt::registry::entity_type entity)
        {
            using Component::MouseMotion;

            auto motion = registry.get<MouseMotion>(entity);

            switch (state)
            {
                case START:
                {
                    if (registry.valid(hovered)) unhighlight(registry, hovered);
                    auto hover = close_enough_to_grab(registry, motion.position);
                    if (hover != entt::null) // got a hover
                    {
                        highlight(registry, hover);
                        hovered = hover;
                    }
                    else hovered = entt::null;
                    break;
                }
                case MAYBE_DRAG:
                    state = DRAG;
                    // fall through
                case DRAG:
                    drag(registry, motion, last_group);
                    break;

                case MAYBE_SELECT_DRAG:
                {
                    auto touched = close_enough_to_grab(registry, motion.position);
                    if (touched == entt::null) break;

                    // if something is touched
                    last_group = group(registry, touched);
                    if (not shift(registry))
                    {
                        unselect_all(registry, last_group);
                        select(registry, touched);
                        touch(registry, touched);
                    }
                    else
                    {
                        toggle_selection(registry, touched);
                        touch(registry, touched);
                    }
                    state = SELECT_DRAG;
                    break;
                }
                    // fall through
                case SELECT_DRAG:
                {
                    auto touched = close_enough_to_grab(registry, motion.position);
                    if (touched == entt::null) break;
                    if (already_touched(registry, touched)) break;
                    if (group(registry, touched) != last_group) break;
                    toggle_selection(registry, touched);
                    touch(registry, touched);
                    break;
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
                    if (not btn.pressed) break;

                    auto grab = close_enough_to_grab(registry, btn.down_position);
                    if (grab != entt::null) // got a grab
                    {
                        last_group = group(registry, grab);
                        if (selected(registry, grab))
                        {
                            maybe_drop = grab;
                            state = MAYBE_DRAG;
                            break;
                        }
                        else
                        {
                            if (not shift(registry)) unselect_all(registry, last_group);
                            select(registry, grab);
                            state = DRAG;
                            break;
                        }
                    }
                    else // no grab
                    {
                        untouch_all(registry);
                        state = MAYBE_SELECT_DRAG;
                        break;
                    }
                    break;
                }

                case MAYBE_DRAG:
                    if (btn.pressed) break; // this should never happen
                    if (shift(registry) && registry.valid(maybe_drop)) unselect(registry, maybe_drop);
                    else 
                    {
                        unselect_all(registry, last_group);
                        select(registry, maybe_drop);
                    }
                    state = START;
                    break;

                case MAYBE_SELECT_DRAG:
                    if (btn.pressed) break; // this should never happen
                    unselect_all(registry, last_group);
                    last_group = Component::Selectable::Group::All;
                    state = START;
                    break;

                case DRAG:
                case SELECT_DRAG:
                    if (btn.pressed) break; // this should never happen
                    state = START;
                    break;
            }
            std::cout << static_cast<int>(last_group) << std::endl;
        }

        void run(entt::registry& registry)
        {
        }
    };

    Draggable::Draggable()
    {
        pimpl = new Implementation();
    }

    void Draggable::setup_reactive_systems(entt::registry& registry)
    {
        pimpl->setup_reactive_systems(registry);
    }

    void Draggable::prepare_registry(entt::registry& registry)
    {
        pimpl->prepare_registry(registry);
    }

    Draggable::~Draggable() { delete pimpl; }

    void Draggable::run(entt::registry& registry) { pimpl->run(registry); }
}
