#include "interpolator.h"
#include <iostream>
#include "components/demo.h"
#include "components/paint_flag.h"
#include "components/mouse_button.h"
#include "components/mouse_motion.h"
#include "components/draggable.h"
#include "components/knob.h"
#include "systems/common/interpolator.h"
#include "entt/entity/entity.hpp"

namespace System
{
    struct Interpolator::Implementation
    {
        bool pressed = false;
        bool hid_override = false;
        Component::Demo::Source cache = Component::Demo::Source::Zero();

        void hid_override_query(entt::registry& registry)
        {
            hid_override = true;
            //// if grabbed knob, play a demo
            //auto grabbed_knob = registry.view<Component::Knob, Component::Grabbed>();
            //for (auto && [entity, knob] : grabbed_knob.each())
            //{
            //    // get first selected demo
            //    entt::entity first_demo = entt::null;
            //    auto first_demo_view = registry.view<Component::Selected, Component::Demo>();
            //    for (auto e : first_demo_view) { first_demo = e; break; }
            //    if (first_demo == entt::null)
            //    {
            //        return; // don't edit context if dragging knob with no sel
            //    }

            //    // play first selected demo's parameters
            //    auto s = registry.get<Component::FMSynthParameters>(first_demo);
            //    registry.set<Component::FMSynthParameters>(s);
            //    return;
            //}
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
            if (button.consumed) return;
            pressed = button.pressed;
            if (pressed) hid_override_query(registry);
        }

        void on_mouse_motion(entt::registry& registry, entt::registry::entity_type entity)
        {
            if (not pressed) return;
            auto motion = registry.get<Component::MouseMotion>(entity);
            hid_override_query(registry);
        }

        void run(entt::registry& registry)
        {
            if (hid_override) hid_override = false;
            else normal_query(registry);
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
