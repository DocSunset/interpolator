#include "interpolator.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/mouse_button.h"
#include "components/mouse_motion.h"
#include "components/fmsynth.h"
#include "components/draggable.h"
#include "components/knob.h"
#include "systems/common/interpolator.h"
#include "entt/entity/entity.hpp"

#include <vector>

namespace
{
    template<typename ViewEachPack>
    Component::Position source(ViewEachPack& pack)
    {
        auto &&[entity, demo] = pack;
        return *demo.source;
    }

    template<typename ViewEachPack>
    Component::FMSynthParameters destination(ViewEachPack& pack)
    {
        auto &&[entity, demo] = pack;
        return *demo.destination;
    }

    template<typename ViewEachPack>
    int id(ViewEachPack& pack)
    {
        auto &&[entity, demo] = pack;
        return demo.id;
    }
}

#include "dataset/interpolators/intersecting_n_spheres.h"
#include "dataset/interpolators/utility/weights.h"

namespace System
{
    struct Interpolator::Implementation
    {
        using Scalar = float;
        bool pressed = false;
        Component::Position position = Component::Position::Zero();

        void run_query(entt::registry& registry)
        {
            // if grabbed knob, play a demo
            auto grabbed_knob = registry.view<Component::Knob, Component::Grabbed>();
            for (auto && [entity, knob] : grabbed_knob.each())
            {
                // get first selected demo
                entt::entity first_demo = entt::null;
                auto first_demo_view = registry.view<Component::Selected, Component::Demo>();
                for (auto e : first_demo_view) { first_demo = e; break; }
                if (first_demo == entt::null) break; // this should never happen

                // play first selected demo's parameters
                auto s = registry.get<Component::FMSynthParameters>(first_demo);
                registry.set<Component::FMSynthParameters>(s);
                return;
            }

            auto s = query(registry, position);
            if (not pressed) set_amplitude(s, 0);
            registry.set<Component::FMSynthParameters>(s);
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
            pressed = button.pressed;
            run_query(registry);
        }

        void on_mouse_motion(entt::registry& registry, entt::registry::entity_type entity)
        {
            auto motion = registry.get<Component::MouseMotion>(entity);
            position = motion.position;
            run_query(registry);
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

    Interpolator::~Interpolator()
    {
        free(pimpl);
    }
}
