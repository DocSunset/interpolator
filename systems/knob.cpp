#include "knob.h"
#include "components/draggable.h"
#include "components/demo.h"
#include "components/knob.h"
#include "components/fmsynth.h"
#include "components/window.h"
#include <iostream>
#include <vector>

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

    void track_selected_demos(entt::registry& registry
            , std::vector<entt::entity>& demo_list
            , entt::observer& selected_demos
            )
    {
        if (selected_demos.size()) demo_list.clear();
        auto layout = [&](entt::entity entity)
        {
            auto selected = registry.get<Component::Selected>(entity);
            if (selected)
            {
                demo_list.push_back(entity);
            }
        };
        selected_demos.each(layout);
    }

    void manage_knob_lifetimes(entt::registry& registry, std::vector<entt::entity>& demo_list)
    {
        auto knobs = registry.view<Component::Knob>();
        if (demo_list.size())
        {
            if (!knobs.size()) for (int i = 0; i < Component::FMSynthParameters::N; ++i)
            {
                auto knob = registry.create();
                registry.emplace<Component::Position>(knob, 0, 100 * i);
                registry.emplace<Component::Selected>(knob, false);
                registry.emplace<Component::SelectionHovered>(knob, false);
                registry.emplace<Component::Draggable>(knob, 75);
                auto p = registry.get<Component::FMSynthParameters>(demo_list[0]);
                registry.emplace<Component::Knob>(knob, p.parameters[i], i);
            }
        }
        else if (knobs.size())
        {
            for (auto knob : knobs)
            {
                registry.destroy(knob);
            }
        }
    }
}

namespace System
{
    struct Knob::Implementation
    {
        entt::observer selected_demos;
        std::vector<entt::entity> demo_list;

        Implementation()
        {
        }

        void setup_reactive_systems(entt::registry& registry)
        {
            registry.on_construct<Component::Knob>().connect<&position_knob>();
            registry.on_update<Component::Window>().connect<&on_window_update>();

            selected_demos.connect(registry, entt::collector
                    .update<Component::Selected>()
                    .where<Component::Demo>()
                    );
        }

        void prepare_registry(entt::registry& registry)
        {
        }

        void run(entt::registry& registry)
        {
            track_selected_demos(registry, demo_list, selected_demos);

            manage_knob_lifetimes(registry, demo_list);
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
