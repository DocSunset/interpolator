#include "knob.h"
#include "components/color.h"
#include "components/draggable.h"
#include "components/demo.h"
#include "components/knob.h"
#include "components/fmsynth.h"
#include "components/window.h"
#include <functional>
#include <iostream>
#include <vector>
#include <numeric>

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

    void manage_knob_lifetimes(entt::registry& registry)
    {
        bool any_selected_demos = false;
        auto knobs = registry.view<Component::Knob>();
        auto selected_demos = registry.view<Component::Demo, Component::Selected>();
        for (auto demo_entity : selected_demos)
        {
            any_selected_demos = true;
            if (!knobs.size()) for (int i = 0; i < Component::FMSynthParameters::N; ++i)
            {
                auto knob = registry.create();
                registry.emplace<Component::Position>(knob, 0, 100 * i);
                registry.emplace<Component::Selectable>(knob, false);
                registry.emplace<Component::Color>(knob);
                registry.emplace<Component::SelectionHovered>(knob, false);
                registry.emplace<Component::Draggable>(knob, 75);
                registry.emplace<Component::Knob>(knob, i);
            }
            break;
        }
        if (!any_selected_demos && knobs.size())
        {
            for (auto knob : knobs)
            {
                registry.destroy(knob);
            }
        }
    }

    void sync_knob_values(entt::registry& registry)
    {
        auto knobs = registry.view<Component::Knob>();
        auto selected_demos = registry.view<Component::Demo, Component::Selected>();

        int n_demos;
        for (auto knob_entity : knobs)
        {
            n_demos = 0;
            auto knob = registry.get<Component::Knob>(knob_entity);
            auto get_param = [&](auto demo_entity)
            {
                ++n_demos;
                auto p = registry.get<Component::FMSynthParameters>(demo_entity);
                return p.parameters[knob.index];
            };
            knob.value = std::transform_reduce
                ( selected_demos.begin(), selected_demos.end()
                , 0.0f, std::plus<float>(), get_param
                );
            knob.value = knob.value / (float)n_demos;
            registry.replace<Component::Knob>(knob_entity, knob);
        }

        auto color = std::transform_reduce
            ( selected_demos.begin(), selected_demos.end()
            , Component::Color{0.0f,0.0f,0.0f,0.0f}
            , [](auto a, auto b){
            std::cout << "color b: " << b.r << " " << b.g << " " << b.b << "\n";
            return Component::Color{a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a};}
            , [&](auto entity){return registry.get<Component::Color>(entity);}
            );

        color = { color.r / (float)n_demos
                , color.g / (float)n_demos
                , color.b / (float)n_demos
                , color.a / (float)n_demos
                };

        for (auto knob_entity : knobs)
            registry.replace<Component::Color>(knob_entity, color);
    }
}

namespace System
{
    struct Knob::Implementation
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
            manage_knob_lifetimes(registry);
            sync_knob_values(registry);
        }
    };

    /* pimpl boilerplate *****************************************/

    Knob::Knob()
    {
        pimpl = new Implementation();
    }

    void Knob::setup_reactive_systems(entt::registry& registry)
    {
        pimpl->setup_reactive_systems(registry);
    }

    void Knob::prepare_registry(entt::registry& registry)
    {
        pimpl->prepare_registry(registry);
    }

    Knob::~Knob()
    {
        free(pimpl);
    }
    
    void Knob::run(entt::registry& registry)
    {
        pimpl->run(registry);
    }
}
