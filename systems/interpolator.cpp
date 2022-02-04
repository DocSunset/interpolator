#include "interpolator.h"

#include "entt/entity/entity.hpp"

#include "components/demo.h"
#include "components/paint_flag.h"
#include "components/mouse_button.h"
#include "components/mouse_motion.h"
#include "components/draggable.h"
#include "components/knob.h"
#include "components/cursor.h"

#include "systems/common/interpolator.h"
#include "systems/common/vis.h"

namespace System
{
    struct Interpolator::Implementation
    {
        bool hid_override = false;
        Component::Demo::Source cache = Component::Demo::Source::Zero();

        void audition(entt::registry& registry, entt::entity entity)
        {
            const auto& d = registry.get<Component::Demo::Destination>(entity);
            registry.set<Component::Demo::Destination>(d);
            registry.ctx<Component::PaintFlag>().set();
        }

        void audition(entt::registry& registry, const Component::Position& position)
        {
            auto source = position_to_source(registry, position);
            auto& destination = registry.ctx<Component::Demo::Destination>();
            destination = query(registry, source);
            registry.ctx<Component::PaintFlag>().set();
        }

        void hid_override_query(entt::registry& registry, const Component::Position& position)
        {
            auto grabbed_knob = registry.view<Component::Knob, Component::Grabbed>();
            for (auto && [entity, knob] : grabbed_knob.each())
            {
                // get first selected demo
                auto selected_demo = registry.view<Component::Selected, Component::Demo>();
                for (auto entity : selected_demo) return audition(registry, entity);
                break;
            }

            // play first grabbed demo; should really be closest grabbed demo...
            auto grabbed_demo = registry.view<Component::Grabbed, Component::Demo>();
            for (auto entity : grabbed_demo) return audition(registry, entity);

            auto grabbed_cursor = registry.view<Component::Grabbed, Component::Cursor>();
            for (auto entity : grabbed_cursor)
                return audition(registry, registry.get<Component::Position>(entity));

            audition(registry, position);
        }

        void normal_query(entt::registry& registry)
        {
            auto& source = registry.ctx<Component::Demo::Source>();
            auto& destination = registry.ctx<Component::Demo::Destination>();
            if (source != cache)
            {
                destination = query(registry, source); 
                cache = source;
                registry.ctx<Component::PaintFlag>().set();
            }
        }

        void setup_reactive_systems(entt::registry& registry)
        {
            registry.on_update<Component::MouseMotion>()
                .connect<&Interpolator::Implementation::on_mouse_motion>(*this);
            registry.on_update<Component::LeftMouseButton>()
                .connect<&Interpolator::Implementation::on_mouse_button>(*this);
        }

        void on_mouse_button(entt::registry& registry, entt::registry::entity_type entity)
        {
            auto button = registry.get<Component::LeftMouseButton>(entity);
            if (not button.consumed && button.pressed)
            {
                hid_override = true;
                hid_override_query(registry, button.down_position);
            }
            else hid_override = false;
        }

        void on_mouse_motion(entt::registry& registry, entt::registry::entity_type entity)
        {
            if (not hid_override) return;
            auto motion = registry.get<Component::MouseMotion>(entity);
            hid_override_query(registry, motion.position);
        }

        void run(entt::registry& registry)
        {
            if (!hid_override) normal_query(registry);
        }
    };

    /* pimpl boilerplate *****************************************/

    void Interpolator::construct_system()
    {
        pimpl = new Implementation();
    }

    void Interpolator::setup_reactive_systems(entt::registry& registry)
    {
        pimpl->setup_reactive_systems(registry);
    }

    void Interpolator::run(entt::registry& registry)
    {
        pimpl->run(registry);
    }

    Interpolator::~Interpolator()
    {
        free(pimpl);
    }
}
