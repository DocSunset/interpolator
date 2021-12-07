#include "interpolator.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/mouse_button.h"
#include "components/mouse_motion.h"
#include "components/fmsynth.h"
#include "components/draggable.h"
#include "components/knob.h"
#include "entt/entity/entity.hpp"

#include <vector>
#include <iostream>

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
        std::vector<Scalar> radius;
        std::vector<Scalar> distance;
        std::vector<Scalar> weight;

        auto query(entt::registry& registry)
        {
            using P = Component::Position;
            using S = Component::FMSynthParameters;

            std::size_t i = 0;
            for (auto && [entity, demo, position, params] : registry.view<Component::Demo, P, S>().each())
            {
                demo.source = &position;
                demo.destination = &params;
                ++i;
            }
            distance.resize(i);
            radius.resize(i);
            weight.resize(i);
            auto demo = registry.view<Component::Demo>().each();
            Scalar dummy;
            ::Interpolator::intersecting_spheres_update<Scalar>(position, demo, radius);
            ::Interpolator::intersecting_spheres_query<Scalar>(position, demo, radius, dummy, distance, weight);
            S s = S::Zero();
            return ::Interpolator::Utility::normalized_weighted_sum<Scalar>(demo, weight, s);
        }

        void run_query(entt::registry& registry)
        {
            using P = Component::Position;
            using S = Component::FMSynthParameters;

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
                auto s = registry.get<S>(first_demo);
                registry.set<S>(s);
                return;
            }

            auto s = query(registry);
            if (not pressed) set_amplitude(s, 0);
            registry.set<S>(s);
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
            position = button.pressed ? button.down_position : button.up_position;
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

    void Interpolator::prepare_registry(entt::registry& registry)
    {
        registry.set<Interpolator*>(this);
    }

    Interpolator::~Interpolator()
    {
        free(pimpl);
    }
}
