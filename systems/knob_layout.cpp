#include "knob_layout.h"
#include "components/knob.h"
#include "components/draggable.h"
#include "components/window.h"

namespace
{
    void position_knob(entt::registry& registry, entt::entity entity)
    {
        constexpr float padding = 5;
        auto knob = registry.get<Component::Knob>(entity);
        auto radius = padding + registry.get<Component::Draggable>(entity).radius;
        auto window = registry.get<Component::Window>(registry.view<Component::Window>()[0]);
        float top_left_x = (window.w / 2.0f) - radius;
        float top_left_y = (window.h / 2.0f) - radius;
        registry.replace<Component::Position>(entity, top_left_x, top_left_y - (knob.index * radius));
    }

    void on_window_update(entt::registry& registry, entt::entity entity)
    {
        auto knobs = registry.view<Component::Knob>();
        for (auto knob : knobs) position_knob(registry, knob);
    }
}

namespace System
{
    struct KnobLayout::Implementation
    {
        Implementation()
        {
        }

        void setup_reactive_systems(entt::registry& registry)
        {
            registry.on_construct<Component::Knob>().connect<&position_knob>();
            registry.on_update<Component::Window>().connect<&on_window_update>();
        }

        void prepare_registry(entt::registry& registry)
        {
        }

        void run(entt::registry& registry)
        {
        }
    };

    /* pimpl boilerplate *****************************************/

    KnobLayout::KnobLayout()
    {
        pimpl = new Implementation();
    }

    void KnobLayout::setup_reactive_systems(entt::registry& registry)
    {
        pimpl->setup_reactive_systems(registry);
    }

    void KnobLayout::prepare_registry(entt::registry& registry)
    {
        pimpl->prepare_registry(registry);
    }

    KnobLayout::~KnobLayout()
    {
        free(pimpl);
    }
    
    void KnobLayout::run(entt::registry& registry)
    {
        pimpl->run(registry);
    }
}
