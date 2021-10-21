#include "knob_maker.h"
#include "components/draggable.h"
#include "components/demo.h"
#include "components/knob.h"
#include "components/fmsynth.h"
#include <iostream>
#include <vector>

namespace System
{
    struct KnobMaker::Implementation
    {
        entt::observer selected_demos;
        std::vector<entt::entity> demo_list;

        Implementation()
        {
        }

        void setup_reactive_systems(entt::registry& registry)
        {
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
            auto knobs = registry.view<Component::Knob>();

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
    };

    /* pimpl boilerplate *****************************************/

    KnobMaker::KnobMaker()
    {
        pimpl = new Implementation();
    }

    void KnobMaker::setup_reactive_systems(entt::registry& registry)
    {
        pimpl->setup_reactive_systems(registry);
    }

    void KnobMaker::prepare_registry(entt::registry& registry)
    {
        pimpl->prepare_registry(registry);
    }

    KnobMaker::~KnobMaker()
    {
        free(pimpl);
    }
    
    void KnobMaker::run(entt::registry& registry)
    {
        pimpl->run(registry);
    }
}
