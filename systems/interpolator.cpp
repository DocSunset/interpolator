#include "interpolator.h"
#include "components/demo.h"
#include "components/position.h"
#include "components/mouse_button.h"
#include "components/mouse_motion.h"
#include "components/fmsynth.h"

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

        Implementation()
        {
        }

        void setup_reactive_systems(entt::registry& registry)
        {
            registry.on_update<Component::MouseMotion>()
                .connect<&Interpolator::Implementation::on_mouse_motion>(*this);
            registry.on_update<Component::LeftMouseButton>()
                .connect<&Interpolator::Implementation::on_mouse_button>(*this);
        }

        void prepare_registry(entt::registry& registry)
        {
        }

        void on_mouse_button(entt::registry& registry, entt::registry::entity_type entity)
        {
            auto button = registry.get<Component::LeftMouseButton>(entity);
            pressed = button.pressed;
            position = button.pressed ? button.down_position : button.up_position;
        }

        void on_mouse_motion(entt::registry& registry, entt::registry::entity_type entity)
        {
            auto motion = registry.get<Component::MouseMotion>(entity);
            position = motion.position;
        }

        void run(entt::registry& registry)
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
            s = ::Interpolator::Utility::normalized_weighted_sum<Scalar>(demo, weight, s);
            if (not pressed) set_amplitude(s, 0);
            registry.set<S>(s);
        }
    };

    /* pimpl boilerplate *****************************************/

    Interpolator::Interpolator()
    {
        pimpl = new Implementation();
    }

    void Interpolator::setup_reactive_systems(entt::registry& registry)
    {
        pimpl->setup_reactive_systems(registry);
    }

    void Interpolator::prepare_registry(entt::registry& registry)
    {
        pimpl->prepare_registry(registry);
    }

    Interpolator::~Interpolator()
    {
        free(pimpl);
    }
    
    void Interpolator::run(entt::registry& registry)
    {
        pimpl->run(registry);
    }
}
