#include "draggable.h"

#include <limits>
#include "components/mouse_button.h"
#include "components/mouse_motion.h"
#include "components/modifier_keys.h"
#include "components/demo.h"
#include "components/knob.h"
#include "components/position.h"
#include "components/draggable.h"
#include "components/fmsynth.h"
#include "components/mouse_mode.h"
#include "components/mouse_position.h"
#include "components/paint_flag.h"
#include "entt/entity/entity.hpp"

namespace
{
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

    /* actions
     */
    void highlight(entt::registry& registry, entt::entity entity)
    {
        bool old_value = registry.get<Component::SelectionHovered>(entity);
        registry.replace<Component::SelectionHovered>(entity, true);
        if (not old_value) registry.ctx<Component::PaintFlag>().set();
    }

    void unhighlight(entt::registry& registry, entt::entity entity)
    {
        bool old_value = registry.get<Component::SelectionHovered>(entity);
        registry.replace<Component::SelectionHovered>(entity, false);
        if (old_value) registry.ctx<Component::PaintFlag>().set();
    }

    void select(entt::registry& registry, entt::entity entity)
    {
        auto s = registry.get<Component::Selectable>(entity);
        registry.replace<Component::Selectable>(entity, true, s.group);
        registry.emplace_or_replace<Component::Selected>(entity);
        registry.ctx<Component::PaintFlag>().set();
    }

    void unselect(entt::registry& registry, entt::entity entity)
    {
        auto s = registry.get<Component::Selectable>(entity);
        registry.replace<Component::Selectable>(entity, false, s.group);
        if (registry.all_of<Component::Selected>(entity))
            registry.erase<Component::Selected>(entity);
        registry.ctx<Component::PaintFlag>().set();
    }

    void toggle_selection(entt::registry& registry, entt::entity entity)
    {
        auto current = registry.get<Component::Selectable>(entity);
        if (current) unselect(registry, entity);
        else select(registry, entity);
        registry.ctx<Component::PaintFlag>().set();
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

    void grab(entt::registry& registry, Component::Selectable::Group grab_group)
    {
        auto view = registry.view<Component::Selected, Component::Selectable, Component::Draggable>();
        for (auto &&[entity, selectable, draggable] : view.each())
        {
            if (group(registry, entity) == grab_group)
                registry.emplace_or_replace<Component::Grabbed>(entity);
        }
    }

    void ungrab(entt::registry& registry)
    {
        registry.clear<Component::Grabbed>();
    }

    void drag(entt::registry& registry, const Component::MouseMotion& motion)
    {
        auto draggables = registry.view<Component::Grabbed, Component::Draggable>();
        for (auto &&[entity, draggable] : draggables.each())
        {
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
        registry.ctx<Component::PaintFlag>().set();
    }

    void set_relative_mouse(entt::registry& registry, bool mode)
    {
        auto mouse_entity = registry.view<Component::RelativeMouseMode>()[0];
        registry.replace<Component::RelativeMouseMode>(mouse_entity, mode);
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
        Component::MousePosition drag_start_position;

        void setup_reactive_systems(entt::registry& registry)
        {
            registry.on_update<Component::MouseMotion>()
                .connect<&Draggable::Implementation::on_mouse_motion>(*this);
            registry.on_update<Component::LeftMouseButton>()
                .connect<&Draggable::Implementation::on_mouse_button>(*this);
        }

        void start_drag(entt::registry& registry)
        {
            if (last_group == Component::Selectable::Group::Knob)
            {
                drag_start_position = registry.get<Component::MousePosition>(
                        registry.view<Component::MousePosition>()[0]);
                set_relative_mouse(registry, true);
            }
            grab(registry, last_group);
            state = DRAG;
        }

        void end_drag(entt::registry& registry)
        {
            if (last_group == Component::Selectable::Group::Knob)
            {
                set_relative_mouse(registry, false);
                registry.replace<Component::MousePosition>(
                        registry.view<Component::MousePosition>()[0],
                        drag_start_position);
            }
            ungrab(registry);
            state = START;
        }

        void on_mouse_motion(entt::registry& registry, entt::registry::entity_type entity)
        {
            using Component::MouseMotion;

            auto motion = registry.get<MouseMotion>(entity);

            switch (state)
            {
                case START:
                {
                    auto hover = close_enough_to_grab(registry, motion.position);
                    if (hover != entt::null) // got a hover
                    {
                        if (registry.valid(hovered) && hover != hovered) 
                            unhighlight(registry, hovered);
                        highlight(registry, hover);
                        hovered = hover;
                    }
                    else 
                    {
                        if (registry.valid(hovered)) unhighlight(registry, hovered);
                        hovered = entt::null;
                    }
                    break;
                }
                case MAYBE_DRAG:
                    start_drag(registry);
                    // fall through
                case DRAG:
                    drag(registry, motion);
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

                    auto grabbed = close_enough_to_grab(registry, btn.down_position);
                    if (grabbed != entt::null) // got grabbed object
                    {
                        last_group = group(registry, grabbed);
                        if (selected(registry, grabbed))
                        {
                            maybe_drop = grabbed;
                            state = MAYBE_DRAG;
                            break;
                        }
                        else
                        {
                            if (not shift(registry)) unselect_all(registry, last_group);
                            select(registry, grabbed);
                            start_drag(registry);
                            break;
                        }
                    }
                    else // no grabbed object
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
                    end_drag(registry);
                    break;
            }
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

    void Draggable::run(entt::registry& registry) { pimpl->run(registry); }

    Draggable::~Draggable() { delete pimpl; }
}
