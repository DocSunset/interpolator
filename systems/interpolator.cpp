#include "interpolator.h"
#include "components/position.h"
#include "components/mouse_button.h"
#include "components/mouse_motion.h"
#include "components/fmsynth.h"

namespace
{
    void run_interpolator(entt::registry& registry, Component::Position position, bool pressed)
    {
        using P = Component::FMSynthParameters;
        auto synth_params = registry.ctx<P>();
        synth_params.amplitude = static_cast<float>(pressed);
        registry.set<P>(synth_params);
    }
}

namespace System
{
    struct Interpolator::Implementation
    {
        bool pressed = false;

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
            run_interpolator(registry
                    , button.pressed ? button.down_position : button.up_position
                    , pressed);
        }

        void on_mouse_motion(entt::registry& registry, entt::registry::entity_type entity)
        {
            auto motion = registry.get<Component::MouseMotion>(entity);
            run_interpolator(registry, motion.position, pressed);
        }

        void run(entt::registry& registry)
        {
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
