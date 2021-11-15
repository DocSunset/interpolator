#pragma once

#include <vector>
#include <memory>
#include <entt/entt.hpp>
#include "systems/system.h"
#include "components/quit_flag.h"
#include "systems/platform.h"
#include "systems/demo_maker.h"
#include "systems/demo_viewer.h"
#include "systems/draggable.h"
#include "systems/demo_dragger.h"
#include "systems/interpolator.h"
#include "systems/knob.h"
#include "systems/knob_viewer.h"
#include "systems/interpolator_visualizer.h"
#include "systems/circle_painter.h"
#include "systems/line_painter.h"

class App
{
    entt::registry registry;
    std::vector<std::unique_ptr<System::System>> systems;

public:
    void loop()
    {
        // system execution order == order in list (established by 2)
        for (auto& system : systems)
        {
            system->run(registry);
        }
    }

    App()
    {
        // system constructor order == execution order
        systems.push_back(std::make_unique<System::Platform>());
        systems.push_back(std::make_unique<System::Draggable>());
        systems.push_back(std::make_unique<System::DemoDragger>());
        systems.push_back(std::make_unique<System::Knob>());
        systems.push_back(std::make_unique<System::Interpolator>());
        systems.push_back(std::make_unique<System::DemoViewer>());
        systems.push_back(std::make_unique<System::KnobViewer>());
        systems.push_back(std::make_unique<System::CirclePainter>());
        systems.push_back(std::make_unique<System::LinePainter>());
        systems.push_back(std::make_unique<System::DemoMaker>());

        for (auto& system : systems) system->setup_reactive_systems(registry);
        for (auto& system : systems) system->prepare_registry(registry);
    }

    ~App()
    {
        // system destructor order == reverse of constructor order
        while (not systems.empty()) systems.pop_back(); 
    }

    bool ready_to_quit() const
    {
        // quit condition is signalled by QuitFlag in registry context
        auto* quit = registry.try_ctx<Component::QuitFlag>();
        if (quit != nullptr && static_cast<bool>(*quit) == true) 
             return true;
        else return false;
    }
};
